// Arduino sketch for a two-wheeled self-balancing robot using LQR control
// with BNO055 IMU module and ODrive motor controller. Tested on Arduino Mega 2560 with ODrive 3.6.

#include <Wire.h>
#include <Metro.h>
#include <Servo.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <ODriveArduino.h>
#include <Command_processor.h>
//////////////////////////////////////////////////////////yeonggwang
#include <ros.h>
#include <sensor_msgs/Imu.h>
#include <nav_msgs/Odometry.h>
////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////memory
#include <MemoryFree.h>
/////////////////////////////////////////

namespace {
// Hardware settings.
constexpr unsigned short kLedPin = 13;
constexpr unsigned short kSteeringPwmInputPin = 2;
constexpr unsigned short kThrottlePwmInputPin = 3;
constexpr unsigned short kEngagePwmInputPin = 18;

constexpr unsigned long kSerialBaudratePc = 115200;
constexpr unsigned long kSerialBaudrateOdrive = 115200;
constexpr int kMotorDir0 = 1; // Can only be 1 or -1.
constexpr int kMotorDir1 = -1; // Can only be 1 or -1.
constexpr int kSteeringPolarity = 1; // Can only be 1 or -1.
constexpr int kThrottlePolarity = 1; // Can only be 1 or -1.

// RC settings.
constexpr int kSteeringPwmOffset = 1492;
constexpr int kThrottlePwmOffset = 1488;
constexpr int kEngageThresholdPwm = 1500;
constexpr int kControlThreshold = 50; // Steering or throttle threshold to disable wheel angle correction

// LQR Gain Matrix (from MATLAB)
float K_phi = 0;
float K_theta = 22.95;
float K_phi_dot = 0.61;
float K_theta_dot = 2.95;
float imu_angle_offset = 2.5;  // IMU 각도 오프셋 값
float kTorqueConstant = 3;

constexpr float kMaxSpeedForward = 2.0; // 직선 최대 속도
constexpr float kMaxSpeedTurn = 1.0;    // 회전 최대 속도 
constexpr float kAlpha = 0.01;  // 필터 강도 (0.0 ~ 1.0 사이, 낮을수록 더 부드러움)
float filtered_throttle_pwm = 0;
float filtered_steering_pwm = 0;
float filtered_engage_pwm = 0;
constexpr float kWheelRadius = 0.0825; // 바퀴 반지름 [m]
constexpr uint8_t kTiltDisengageThresholdDegrees = 40; // 전원이 차단되는 각도 임계값
constexpr float kMaxAbsCurrent = 4.0;
constexpr int kCpr = 90; // 홀센서 Counts Per Revolution (CPR)

// Task scheduling settings.
constexpr unsigned int kBlinkIntervalMs = 200;
constexpr unsigned int kControllerIntervalMs = 10;
constexpr unsigned int kActivationIntervalMs = 50;
constexpr unsigned int kPrintIntervalMs = 50;

unsigned long loop_start_time;
unsigned long loop_duration;

// State variables.
bool motors_active = false;
bool tilt_limit_exceeded = false;
int engage_signal_persistence = 0;
bool engage = false;

// PWM decoder variables.
unsigned long last_throttle_pwm_rise_time = 0;
unsigned long last_steering_pwm_rise_time = 0;
unsigned long last_engage_pwm_rise_time = 0;
int throttle_pwm = 0;
int steering_pwm = 0;
int engage_pwm = 0;

float encoder_offset_0 = 0.0;
float encoder_offset_1 = 0.0;
bool was_driving = false;
bool initialized = false;
    // === 추가할 변수 ===
int drive_signal_persistence = 0;  // 주행 상태 필터링 변수
constexpr int kPersistenceThreshold = 3;  // 신호 지속성 임계값 (3번 이상 유지될 때만 동작)

// Interactive flags
bool imu_enabled = true;
bool rc_enabled = true;
bool rc_print_enabled = false;
bool motion_controller_enabled = true;

Adafruit_BNO055 bno = Adafruit_BNO055();
/////////////////////////////////////////////////////yeonggwang
//ros node handle
ros::NodeHandle nh;
sensor_msgs::Imu imu_msg;
nav_msgs::Odometry odom_msg;
ros::Publisher imu_pub("/imu", &imu_msg);
ros::Publisher odom_pub("/odom", &odom_msg);

const float wheel_radius = 0.0827;  // 휠 반지름 (m)
const float wheel_base = 0.465;     // 휠 베이스 (m)
float x = 0.0, y = 0.0, theta = 0.0;
unsigned long last_time = 0;
const float pi = 3.14159265358979323846;

////////////////////////////////////////////////////////////////
ODriveArduino odrive(Serial2);

// Instantiate Metros for task scheduling.
Metro led_metro = Metro(kBlinkIntervalMs);
Metro controller_metro = Metro(kControllerIntervalMs);
Metro activation_metro = Metro(kActivationIntervalMs);
Metro print_metro = Metro(kPrintIntervalMs);
Command_processor cmd;
}

#define N 5  // 최근 N개의 속도 변화 저장 개수
float velocity_history_0[N] = {0};  // 모터 0 속도 변화 저장
float velocity_history_1[N] = {0};  // 모터 1 속도 변화 저장
float raw_history_0[N] = {0};  // 모터 0 위치 변화 저장
float raw_history_1[N] = {0};  // 모터 1 위치 변화 저장
int index = 0;  // 순환 인덱스

// 중간값 필터 함수
float getMedian(float a, float b, float c) {
    if ((a <= b && b <= c) || (c <= b && b <= a)) return b;
    else if ((b <= a && a <= c) || (c <= a && a <= b)) return a;
    else return c;
}

// 동적 Threshold 업데이트 함수
float updateThreshold(float new_value, float *history) {
    history[index] = abs(new_value);  // 새로운 값 저장
    float sum = 0;
    for (int i = 0; i < N; i++) {
        sum += history[i];
    }
    return 3.0 * (sum / N);  // 최근 N개의 평균 값의 3배를 Threshold로 설정
}

void setup() {
  pinMode(kLedPin, OUTPUT);
  Serial.begin(kSerialBaudratePc);
  Serial2.begin(kSerialBaudrateOdrive);
  bno.setExtCrystalUse(true);
  if (!bno.begin()) {
    Serial.println("No BNO055 detected. Halting for safety.");
    while (true);
  }
    //////////////////////////////////////////////yeonggwang
  // ROS
  //nh.initNode();
  //nh.advertise(imu_pub);
  // nh.advertise(odom_pub);
  ////////////////////////////////////////////////////////
  delay(1000);

  // Set ODrive to closed-loop control mode at startup
  odrive.run_state(0, AXIS_STATE_CLOSED_LOOP_CONTROL, false);
  odrive.run_state(1, AXIS_STATE_CLOSED_LOOP_CONTROL, false);

  // PWM decoder interrupt handling.
  attachInterrupt(digitalPinToInterrupt(kSteeringPwmInputPin), SteeringPwmCallbackWrapper, CHANGE);
  attachInterrupt(digitalPinToInterrupt(kThrottlePwmInputPin), ThrottlePwmCallbackWrapper, CHANGE);
  attachInterrupt(digitalPinToInterrupt(kEngagePwmInputPin), EngagePwmCallbackWrapper, CHANGE);

  // Add commands to adjust parameters
  cmd.add_command('q', &SetK_phi, 1, "Set K_phi gain.");
  cmd.add_command('w', &SetK_theta, 1, "Set K_theta gain.");
  cmd.add_command('e', &SetK_phi_dot, 1, "Set K_phi_dot gain.");
  cmd.add_command('r', &SetK_theta_dot, 1, "Set K_theta_dot gain.");
  cmd.add_command('t', &SetImuAngleOffset, 1, "Set IMU angle offset.");
  cmd.add_command('y', &SetTorqueConstant, 1, "Set torque constant.");
}
////////////////////////////////////////////////////////////yeonggwang/////////
void publishimuData() {
  // IMU 메시지 헤더 설정
  imu_msg.header.stamp = nh.now();
  imu_msg.header.frame_id = "base_link";
  imu::Vector<3> acceleration = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
  // 선형 가속도 (m/s^2)
  imu_msg.linear_acceleration.x = acceleration.x();
  imu_msg.linear_acceleration.y = acceleration.y();
  imu_msg.linear_acceleration.z = acceleration.z();

  imu_msg.linear_acceleration_covariance[0] = 0.04;
  imu_msg.linear_acceleration_covariance[1] = 0;
  imu_msg.linear_acceleration_covariance[2] = 0;
  imu_msg.linear_acceleration_covariance[3] = 0;
  imu_msg.linear_acceleration_covariance[4] = 0.04;
  imu_msg.linear_acceleration_covariance[5] = 0;
  imu_msg.linear_acceleration_covariance[6] = 0;
  imu_msg.linear_acceleration_covariance[7] = 0;
  imu_msg.linear_acceleration_covariance[8] = 0.04;
  imu::Vector<3> gyro_rates = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);

  // 각속도 (deg/s -> rad/s)
  imu_msg.angular_velocity.x = gyro_rates.x() * DEG_TO_RAD;
  imu_msg.angular_velocity.y = gyro_rates.y() * DEG_TO_RAD;
  imu_msg.angular_velocity.z = gyro_rates.z() * DEG_TO_RAD;

  imu_msg.angular_velocity_covariance[0] = 0.02;
  imu_msg.angular_velocity_covariance[1] = 0;
  imu_msg.angular_velocity_covariance[2] = 0;
  imu_msg.angular_velocity_covariance[3] = 0;
  imu_msg.angular_velocity_covariance[4] = 0.02;
  imu_msg.angular_velocity_covariance[5] = 0;
  imu_msg.angular_velocity_covariance[6] = 0;
  imu_msg.angular_velocity_covariance[7] = 0;
  imu_msg.angular_velocity_covariance[8] = 0.02;

  imu::Vector<3> euler_angles = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  // Euler 각도를 쿼터니언으로 변환
  float roll = euler_angles.x() * DEG_TO_RAD;   // Roll
  float pitch = euler_angles.y() * DEG_TO_RAD;  // Pitch
  float yaw = euler_angles.z() * DEG_TO_RAD;    // Yaw

  float cy = cos(yaw * 0.5);
  float sy = sin(yaw * 0.5);
  float cp = cos(pitch * 0.5);
  float sp = sin(pitch * 0.5);
  float cr = cos(roll * 0.5);
  float sr = sin(roll * 0.5);
  // 자세 (orientation)
  imu_msg.orientation.w = cr * cp * cy + sr * sp * sy;
  imu_msg.orientation.x = sr * cp * cy - cr * sp * sy;
  imu_msg.orientation.y = cr * sp * cy + sr * cp * sy;
  imu_msg.orientation.z = cr * cp * sy - sr * sp * cy;

  imu_msg.orientation_covariance[0] = 0.0025;
  imu_msg.orientation_covariance[1] = 0;
  imu_msg.orientation_covariance[2] = 0;
  imu_msg.orientation_covariance[3] = 0;
  imu_msg.orientation_covariance[4] = 0.0025;
  imu_msg.orientation_covariance[5] = 0;
  imu_msg.orientation_covariance[6] = 0;
  imu_msg.orientation_covariance[7] = 0;
  imu_msg.orientation_covariance[8] = 0.0025;
  // ROS로 IMU 메시지 전송
  imu_pub.publish(&imu_msg);
}
void publishodomData() {
  float current_pose_left = odrive.GetPosition(0) * kMotorDir0;
  float current_pose_right = odrive.GetPosition(1) * kMotorDir1;
  float current_speed_left = odrive.GetVelocity(0) * kMotorDir0;
  float current_speed_right = odrive.GetVelocity(1) * kMotorDir1;

  float dt = (millis() - last_time) / 1000.0;  // 초 단위로 변환
  last_time = millis();

  float linear_velocity = (current_speed_left + current_speed_right) / 2.0 *pi* 2.0 * wheel_radius;
  float angular_velocity = (current_speed_right - current_speed_left) / wheel_base*pi* 2.0 * wheel_radius;

  theta += angular_velocity * dt;
  x += linear_velocity * cos(theta) * dt;
  y += linear_velocity * sin(theta) * dt;
  // ROS 메시지 구성
  odom_msg.header.stamp = nh.now();
  odom_msg.header.frame_id = "odom";
  odom_msg.child_frame_id = "base_footprint";

  odom_msg.pose.pose.position.x = x;
  odom_msg.pose.pose.position.y = y;
  odom_msg.pose.pose.position.z = 0.0;

  odom_msg.pose.pose.orientation.w = cos(theta / 2.0);
  odom_msg.pose.pose.orientation.z = sin(theta / 2.0);

  odom_msg.twist.twist.linear.x = linear_velocity;
  odom_msg.twist.twist.angular.z = angular_velocity;
  // ROS 퍼블리시
  odom_pub.publish(&odom_msg);
}
////////////////////////////////////////////////////////////////////////////
void loop() {
  //////////////////////////////////////////////////////////yeonggwang
  static uint32_t pre_time;
  if (millis() - pre_time >= 10) {
    pre_time = millis();
    //publishimuData();   // IMU 데이터 퍼블리시
    // publishodomData();  // odom 데이터 퍼블리시
    //nh.spinOnce();      // ROS 메시지 처리
  }
Serial.println("1 ");
  //////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////memory
    Serial.print("freeMemory()=");
    Serial.println(freeMemory());

    delay(500);
  ///////////////////////////////////////////////////////////////////////
  loop_start_time = micros();

  // 저역통과 필터 적용
  filtered_throttle_pwm = kAlpha * throttle_pwm + (1.0 - kAlpha) * filtered_throttle_pwm;
  filtered_steering_pwm = kAlpha * steering_pwm + (1.0 - kAlpha) * filtered_steering_pwm;
  filtered_engage_pwm = kAlpha * engage_pwm + (1.0 - kAlpha) * filtered_engage_pwm;

  if (controller_metro.check()) {
    if (motion_controller_enabled) {
      LqrController();
    }
  }
  if (print_metro.check()) {
    if (rc_print_enabled) {
      PrintRcSignals();
    }
  }
  if (activation_metro.check()) {    // 지속성 필터링
    if (filtered_engage_pwm > kEngageThresholdPwm) {
      ++engage_signal_persistence;
    } else {
      --engage_signal_persistence;
    }
    engage_signal_persistence = constrain(engage_signal_persistence, -2, 2);
    engage = engage_signal_persistence > 0 ? true : false;

    if (engage && !motors_active) {  
    Serial.println("Controller engaged. Motors activated.");  // 전원 켜짐 메시지
    } 
    if (!engage && motors_active) {
    Serial.println("Controller disengaged. Motors deactivated.");  // 전원 꺼짐 메시지
    initialized = false;
    }
    motors_active = engage && !tilt_limit_exceeded;
  }
  if (!motors_active || tilt_limit_exceeded) {
    odrive.SetCurrent(0, 0);  // 전류를 0으로 설정하여 모터 정지
    odrive.SetCurrent(1, 0);
  }
  if (led_metro.check()) {
    digitalWrite(kLedPin, !digitalRead(kLedPin));
  }
  cmd.parse_command();

  // 루프 끝에서 시간 차이 계산 및 출력
  loop_duration = micros() - loop_start_time;
  // Serial.print("Loop duration: ");
  // Serial.print(loop_duration);
  // Serial.println(" us");
}

// LQR Controller Implementation
void LqrController() {
  imu::Vector<3> euler_angles = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  imu::Vector<3> gyro_rates = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);

  // Update the tilt safety flag based on the latest IMU reading.
  if (abs(euler_angles.y() + imu_angle_offset) > kTiltDisengageThresholdDegrees) {
    tilt_limit_exceeded = true;
    return;
  } else {
    tilt_limit_exceeded = false;
  }

  if (!motors_active) return;

  // Body tilt angle and angular velocity
  float theta = imu_enabled * ((euler_angles.y() + imu_angle_offset) * PI / 180.0);
  float theta_dot = imu_enabled * (gyro_rates.y() * PI / 180.0);

  // === 1. 버퍼 초기화 ===
while (Serial2.available()) Serial2.read();  // 이전 데이터 제거

// === 2. ODrive에 4개 값 요청 (위치 2개, 속도 2개) ===
Serial2.print("r axis0.encoder.pos_estimate\n"
              "r axis1.encoder.pos_estimate\n"
              "r axis0.encoder.vel_estimate\n"
              "r axis1.encoder.vel_estimate\n"
              "r axis1.encoder.vel_estimate\n"); //문제 해결용 추가 명령어

// 1️⃣ 센서 값 읽기
    float phi_raw_counts_0 = Serial2.parseFloat();
    float phi_raw_counts_1 = Serial2.parseFloat();
    float phi_dot_counts_0 = Serial2.parseFloat();
    float phi_dot_counts_1 = Serial2.parseFloat();

    // 2️⃣ Adaptive Threshold 계산 (위치 & 속도 모두 적용)
    float adaptive_threshold_raw_0 = updateThreshold(phi_raw_counts_0, raw_history_0);
    float adaptive_threshold_raw_1 = updateThreshold(phi_raw_counts_1, raw_history_1);
    float adaptive_threshold_dot_0 = updateThreshold(phi_dot_counts_0, velocity_history_0);
    float adaptive_threshold_dot_1 = updateThreshold(phi_dot_counts_1, velocity_history_1);

    // 3️⃣ 중간값 필터 적용 (이전 두 값과 현재 값 비교)
    static float prev_raw_0 = 0.0, prev_prev_raw_0 = 0.0;
    static float prev_raw_1 = 0.0, prev_prev_raw_1 = 0.0;
    static float prev_dot_0 = 0.0, prev_prev_dot_0 = 0.0;
    static float prev_dot_1 = 0.0, prev_prev_dot_1 = 0.0;

    float median_phi_raw_0 = getMedian(prev_prev_raw_0, prev_raw_0, phi_raw_counts_0);
    float median_phi_raw_1 = getMedian(prev_prev_raw_1, prev_raw_1, phi_raw_counts_1);
    float median_phi_dot_0 = getMedian(prev_prev_dot_0, prev_dot_0, phi_dot_counts_0);
    float median_phi_dot_1 = getMedian(prev_prev_dot_1, prev_dot_1, phi_dot_counts_1);

    // 4️⃣ Threshold-Based Smoothing 적용 (갑자기 튀는 값 완화)
    if (abs(phi_raw_counts_0 - prev_raw_0) > adaptive_threshold_raw_0) {
        phi_raw_counts_0 = median_phi_raw_0;
    }
    if (abs(phi_raw_counts_1 - prev_raw_1) > adaptive_threshold_raw_1) {
        phi_raw_counts_1 = median_phi_raw_1;
    }
    if (abs(phi_dot_counts_0 - prev_dot_0) > adaptive_threshold_dot_0) {
        phi_dot_counts_0 = median_phi_dot_0;
    }
    if (abs(phi_dot_counts_1 - prev_dot_1) > adaptive_threshold_dot_1) {
        phi_dot_counts_1 = median_phi_dot_1;
    }

    // 5️⃣ 값 업데이트 (다음 루프를 위한 저장)
    prev_prev_raw_0 = prev_raw_0;
    prev_raw_0 = phi_raw_counts_0;

    prev_prev_raw_1 = prev_raw_1;
    prev_raw_1 = phi_raw_counts_1;

    prev_prev_dot_0 = prev_dot_0;
    prev_dot_0 = phi_dot_counts_0;

    prev_prev_dot_1 = prev_dot_1;
    prev_dot_1 = phi_dot_counts_1;

    index = (index + 1) % N;  // 순환 인덱스 업데이트

while (Serial2.available()) Serial2.read();

// === 컨트롤러 전원 켜질 때 초기화 ===
  if (engage && !initialized) {
    encoder_offset_0 = phi_raw_counts_0;  // 초기화 시점의 엔코더 값 저장
    encoder_offset_1 = phi_raw_counts_1;
    initialized = true;  // 초기화가 완료되었음을 표시
  }

// === 주행 중 정지 시 초기화 ===
bool previous_was_driving = was_driving;  // 이전 상태 저장

if (abs(throttle_pwm - kThrottlePwmOffset) > kControlThreshold ||
    abs(steering_pwm - kSteeringPwmOffset) > kControlThreshold) {
  ++drive_signal_persistence;
} else {
  --drive_signal_persistence;
}

// 지속성 필터로 주행 상태 판별
drive_signal_persistence = constrain(drive_signal_persistence, -3, 3);
was_driving = drive_signal_persistence > 0 ? true : false;

// 주행 → 정지로 전환될 때만 초기화 수행
if (previous_was_driving && !was_driving) {
  encoder_offset_0 = phi_raw_counts_0;  // 정지 시점의 엔코더 값 저장
  encoder_offset_1 = phi_raw_counts_1;
  Serial.println("Stopped. Encoder initialized.");  // 디버깅 메시지
}

  float phi_corrected_0 = kMotorDir0 * (phi_raw_counts_0 - encoder_offset_0) * 2 * PI;
  float phi_corrected_1 = kMotorDir1 * (phi_raw_counts_1 - encoder_offset_1) * 2 * PI;
    // 모터 방향을 반영하여 선속도 계산
  float phi_dot_0 = kMotorDir0 * phi_dot_counts_0 * 2 * PI * kWheelRadius;  // 첫 번째 바퀴 속도 (m/s)
  float phi_dot_1 = kMotorDir1 * phi_dot_counts_1 * 2 * PI * kWheelRadius;  // 두 번째 바퀴 속도 (m/s)

  // === 직진 스로틀 입력 처리 ===
float throttle_input;
if (abs(filtered_throttle_pwm - kThrottlePwmOffset) < kControlThreshold) {
    throttle_input = 0.0;  // 제로존 내에서는 0으로 설정
} else if (filtered_throttle_pwm >= kThrottlePwmOffset) {
    throttle_input = (filtered_throttle_pwm - kThrottlePwmOffset) / 512.0;
} else {
    throttle_input = (filtered_throttle_pwm - kThrottlePwmOffset) / 488.0;
}
throttle_input = constrain(throttle_input, -1.0, 1.0);
float forward_speed = throttle_input * kMaxSpeedForward;

// === 회전 스티어링 입력 처리 ===
float steering_input;
if (abs(filtered_steering_pwm - kSteeringPwmOffset) < kControlThreshold) {
    steering_input = 0.0;  // 제로존 내에서는 0으로 설정
} else if (filtered_steering_pwm >= kSteeringPwmOffset) {
    steering_input = (filtered_steering_pwm - kSteeringPwmOffset) / 508.0;
} else {
    steering_input = (filtered_steering_pwm - kSteeringPwmOffset) / 492.0;
}
steering_input = constrain(steering_input, -1.0, 1.0);
float turning_speed = steering_input * kMaxSpeedTurn;

  // === 바퀴별 속도 계산 (직진 + 회전) ===
  float desired_speed_0 = forward_speed - turning_speed;
  float desired_speed_1 = forward_speed + turning_speed;

  float current_K_phi = (was_driving) ? 0.0 : K_phi;

  float torque_input_0 = (-current_K_phi * phi_corrected_0 + K_theta * theta + K_phi_dot * (desired_speed_0 - phi_dot_0) - K_theta_dot * theta_dot);
  float torque_input_1 = (-current_K_phi * phi_corrected_1 + K_theta * theta + K_phi_dot * (desired_speed_1 - phi_dot_1) - K_theta_dot * theta_dot);

  float current_command_0 = torque_input_0 / kTorqueConstant;
  float current_command_1 = torque_input_1 / kTorqueConstant;

  current_command_0 = constrain(current_command_0, -kMaxAbsCurrent, kMaxAbsCurrent);
  current_command_1 = constrain(current_command_1, -kMaxAbsCurrent, kMaxAbsCurrent);

  odrive.SetCurrent(0, kMotorDir0 * current_command_0);
  odrive.SetCurrent(1, kMotorDir1 * current_command_1);

  // === 5. 수신된 값 출력 (명칭 없이 값만 출력) ===
  Serial.print(phi_corrected_0); Serial.print(" ");
  Serial.print(phi_corrected_1); Serial.print(" ");
  Serial.print(phi_dot_0); Serial.print(" ");
  Serial.print(phi_dot_1); Serial.print(" ");
  Serial.print(theta); Serial.print(" ");
  Serial.print(current_command_0); Serial.print(" ");
  Serial.println(current_command_1);

}

void PwmInterruptCallback(unsigned long &last_rise_time, int &pwm_us, int pwm_input_pin) {
  if (digitalRead(pwm_input_pin)) {
    last_rise_time = micros();
  } else {
    pwm_us = micros() - last_rise_time;
  }
}

void ThrottlePwmCallbackWrapper() {
  PwmInterruptCallback(last_throttle_pwm_rise_time, throttle_pwm, kThrottlePwmInputPin);
}

void SteeringPwmCallbackWrapper() {
  PwmInterruptCallback(last_steering_pwm_rise_time, steering_pwm, kSteeringPwmInputPin);
}

void EngagePwmCallbackWrapper() {
  PwmInterruptCallback(last_engage_pwm_rise_time, engage_pwm, kEngagePwmInputPin);
}

void PrintRcSignals() {
  Serial.print(filtered_throttle_pwm);
  Serial.print('\t');
  Serial.print(filtered_steering_pwm);
  Serial.print('\t');
  Serial.print(filtered_engage_pwm);
  Serial.println();
}

// Command functions
void SetK_phi(float value, float foo) {
  Serial.println("Setting K_phi to " + String(value));
  K_phi = value;
}

void SetK_theta(float value, float foo) {
  Serial.println("Setting K_theta to " + String(value));
  K_theta = value;
}

void SetK_phi_dot(float value, float foo) {
  Serial.println("Setting K_phi_dot to " + String(value));
  K_phi_dot = value;
}

void SetK_theta_dot(float value, float foo) {
  Serial.println("Setting K_theta_dot to " + String(value));
  K_theta_dot = value;
}

void SetImuAngleOffset(float value, float foo) {
  Serial.println("Setting IMU angle offset to " + String(value));
  imu_angle_offset = value;
}

void SetTorqueConstant(float value, float foo) {
  Serial.println("Setting Torque Constant to " + String(value));
  kTorqueConstant = value;
}


