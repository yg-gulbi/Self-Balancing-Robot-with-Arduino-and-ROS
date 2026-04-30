
// Arduino sketch for a radio controlled twho-wheeled self balancing robot
// using a BNO055 IMU module and an ODrive motor controller. Tested on
// Arduino Mega 2560 with ODrive 3.6.

#include <Wire.h>
#include <Metro.h>
#include <Servo.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <ODriveArduino.h>
#include <Command_processor.h>

////////////////////////////////////////////////////////////////
#include <ros.h>
#include <sensor_msgs/Imu.h>
////////////////////////////////////////////////////////////////

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
constexpr int kSteeringPwmOffset = 1492; // Change this if your robot turns in place without steering input.
constexpr int kThrottlePwmOffset = 1488; // Change this if your robot (always) drifts forward or backward without throttle input.
constexpr int kEngageThresholdPwm = 1500;

// Controller settings.
float kpBalance = 0; // Refer to the /tests/readme for tuning.
float kdBalance = -1.7; // Refer to the /tests/readme for tuning.
float kpThrottle = 0.0025; // Change this to control how sensitive your robot reacts to throttle input (higher value means more sensitive).
float kpSteer = 0.001; // Change this to control how sensitive your robot reacts to steering input (higher value means more sensitive).
float kdSteer = 0.004; // Change this to control how well your robot tracks a straight line (higher value means it will track better, but react less to steering input).
constexpr uint8_t kTiltDisengageThresholdDegrees = 40; // Tilt angle (in degrees) at which the motors will automatically disable.
constexpr float kMaxAbsCurrent = 4; // Maximum current (absolute) that the Arduino can command to the ODrive.
float kiBalance = 0;        // I 제어 게인
float integralError = 0;    // 누적된 오차
float maxIntegral = 0.1;   // I 항의 최대값 (누적 과도 방지)
float initial_balance = 0; // 토크 초기값
float inertia = 0.02; // 관성 모멘트 (kg·m^2)
float prev_pitch_rate = 0;       // 이전 루프에서의 각속도

constexpr float kpSync = 0;      // 동기화 P 게인
constexpr float kdSync = 0;      // 동기화 D 게인
float prev_command_right = 0;
float prev_command_left = 0;
float target_command_right = 0;
float target_command_left = 0;
float speed_torque = 0;
constexpr float tolerance = 0.1; // 목표 전류 도달 여부 판단 오차
constexpr int steering_threshold = 100; // 회전 감지 임계값
float kpmgl = 14; // mgl에 곱해질 비율, 기본값 설정
float target_pitch = -1.35; // 초기값: 앞쪽 보정 가중치
float speed_error_left = 0;
float speed_error_right = 0;
float target_speed = 0;
float max_pitch_angle = 10.0;// 각도 관련 변수
float speed_p_gain = 1.5; //속도 제어 P
float max_speed_change = 0.04; // 한 루프에서 최대 pitch 변화량 (도)
float deadband_threshold = 0; // 데드밴드 범위 ±0.5도
float scale_factor = 1.0; // 기본값

// Task scheduling settings.
constexpr unsigned int kBlinkIntervalMs = 200;
constexpr unsigned int kControllerIntervalMs = 20;
constexpr unsigned int kActivationIntervalMs = 50;
constexpr unsigned int kPrintIntervalMs = 50;

// State variables.
bool motors_active = false; // motor state
bool tilt_limit_exceeded = false;
int engage_signal_persistence = 0;

// PWM decoder variables.
unsigned long last_throttle_pwm_rise_time = 0;
unsigned long last_steering_pwm_rise_time = 0;
unsigned long last_engage_pwm_rise_time = 0;
int throttle_pwm = 0;
int steering_pwm = 0;
int engage_pwm = 0;

// Interactive flags
bool imu_enabled = true;
bool rc_enabled = true;
bool rc_print_enabled = false;
bool motion_controller_enabled = true;


Adafruit_BNO055 bno = Adafruit_BNO055();
////////////////////////////////////////////////////////////////
//ros node handle
ros::NodeHandle nh;
sensor_msgs::Imu imu_msg;
ros::Publisher imu_pub("/imu", &imu_msg);
const unsigned long imu_publish_interval = 100; // 100ms = 10Hz
unsigned long last_publish_time = 0;
////////////////////////////////////////////////////////////////
  
ODriveArduino odrive(Serial2);

// Instantiate Metros for task scheduling.
Metro led_metro = Metro(kBlinkIntervalMs);
Metro controller_metro = Metro(kControllerIntervalMs);
Metro activation_metro = Metro(kActivationIntervalMs);
Metro print_metro = Metro(kPrintIntervalMs);
Command_processor cmd;
}

void setup() {
  pinMode(kLedPin, OUTPUT);
  Serial.begin(kSerialBaudratePc); // Serial connection to PC.
  Serial2.begin(kSerialBaudrateOdrive); // Serial connection to ODrive.
  bno.setExtCrystalUse(true);
  if (!bno.begin()) {
    // BNO initialization failed. Possible reasons are 1) bad wiring or 2) incorrect I2C address.
    Serial.println("No BNO055 detected. Halting for safety.");
    while (true);
  }
  /////////////////////////////////////////////////////////
  // ROS
  nh.initNode();
  nh.advertise(imu_pub);
  bno.setExtCrystalUse(true); // 
  ////////////////////////////////////////////////////////

  // BNO055
  bno.setMode(Adafruit_BNO055::OPERATION_MODE_NDOF);
  delay(1000);

  // PWM decoder interrupt handling.
  last_throttle_pwm_rise_time = micros();
  last_steering_pwm_rise_time = micros();
  last_engage_pwm_rise_time = micros();
  attachInterrupt(digitalPinToInterrupt(kSteeringPwmInputPin), SteeringPwmCallbackWrapper, CHANGE);
  attachInterrupt(digitalPinToInterrupt(kThrottlePwmInputPin), ThrottlePwmCallbackWrapper, CHANGE);
  attachInterrupt(digitalPinToInterrupt(kEngagePwmInputPin), EngagePwmCallbackWrapper, CHANGE);

  // Check paramters for validity.
  bool parametrs_valid = ((abs(kMotorDir0) == 1) &&
                          (abs(kMotorDir1) == 1) &&
                          (abs(kSteeringPolarity) == 1) &&
                          (abs(kThrottlePolarity) == 1));
  if (!parametrs_valid) {
    Serial.println("Invalid parameters found. Halting for safety.");
    while (true);
  }

  cmd.add_command('p', &SetKpBalance, 1, "Set balance kp gain.");
  cmd.add_command('d', &SetKdBalance, 1, "Set balance kd gain.");
  cmd.add_command('r', &PrintControllerParameters, 0, "Print all controller parameters.");
  cmd.add_command('w', &EnableImu, 0, "Enable/disable IMU.");
  cmd.add_command('t', &EnableRcPrint, 0, "Enable/disable periodic PWM printout.");
  cmd.add_command('u', &EnableRcControl, 0, "Enable/disable RC control.");
  cmd.add_command('k', &EnableMotionController, 0, "Enable/disable motion controller.");
  cmd.add_command('j', &SetInitialBalance, 1, "Set initial balance value.");
  cmd.add_command('f', &Setkpmgl, 1, "Set kpmgl factor.");
  cmd.add_command('g', &SetTargetPitch, 1, "Set target pitch.");
  cmd.add_command('i', &SetKiBalance, 1, "Set balance ki gain.");
  cmd.add_command('o', &SetMaxIntegral, 1, "Set maximum integral limit.");
  cmd.add_command('s', &SetInertia, 1, "Set inertia value."); // 관성 모멘트 설정 명령어 추가
}

void PrintMotorSpeedsForGraph() {
  // 두 모터 속도 가져오기
  float left_motor_speed = odrive.GetVelocity(0);  // 왼쪽 모터 속도
  float right_motor_speed = odrive.GetVelocity(1); // 오른쪽 모터 속도

  // 오른쪽 모터의 방향 반전 (같은 방향으로 표현하기 위해)
  float right_motor_speed_corrected = right_motor_speed * kMotorDir1;
  float left_motor_speed_corrected = left_motor_speed * kMotorDir0;

  // // Serial Plotter에서 읽을 수 있는 형식으로 출력
  // Serial.print(left_motor_speed_corrected); // 왼쪽 모터 속도 출력
  // Serial.print("\t");                       // 탭으로 구분
  // Serial.println(right_motor_speed_corrected); // 오른쪽 모터 속도 출력
}

void loop() {

  ////////////////////////////////////////////////////////////////////////////
  nh.spinOnce();
  unsigned long current_time = millis();
  if (current_time - last_publish_time >= imu_publish_interval) {
    last_publish_time = current_time;

    // IMU 데이터 읽기
    sensors_event_t event;
    bno.getEvent(&event);

    // IMU 메시지 채우기
    imu_msg.header.stamp = nh.now();
    imu_msg.header.frame_id = "imu_link";
    imu_msg.linear_acceleration.x = event.acceleration.x;
    imu_msg.linear_acceleration.y = event.acceleration.y;
    imu_msg.linear_acceleration.z = event.acceleration.z;
    imu_msg.angular_velocity.x = event.gyro.x;
    imu_msg.angular_velocity.y = event.gyro.y;
    imu_msg.angular_velocity.z = event.gyro.z;
    imu_msg.orientation.x = event.orientation.x;
    imu_msg.orientation.y = event.orientation.y;
    imu_msg.orientation.z = event.orientation.z;
    imu_msg.orientation.w = event.orientation.w;

    // ROS
    imu_pub.publish(&imu_msg);
  }
  //////////////////////////////////////////////////////////////////////
  // IMU 메시지 퍼블리시
  imu_pub.publish(&imu_msg);
  if (controller_metro.check()) {
    if (motion_controller_enabled) {
      MotionController();
    }
  }
  if (print_metro.check()) {
    if (rc_print_enabled) {
      PrintRcSignals();
    }
    // 그래프 출력을 위해 두 모터 속도 출력
    // PrintMotorSpeedsForGraph();
  }
  if (activation_metro.check()) {
    // Avoid false positive disengagements due to noisy PWM by persistence filtering.
    if (engage_pwm > kEngageThresholdPwm) {
      ++ engage_signal_persistence;
    }
    else {
      -- engage_signal_persistence;
    }
    engage_signal_persistence = constrain (engage_signal_persistence, -2, 2);
    bool engage = engage_signal_persistence > 0 ? true : false;
    bool request_motor_activation = engage && !tilt_limit_exceeded;
    EngageMotors(request_motor_activation);
  }
  if (led_metro.check()) {
    digitalWrite(kLedPin, !digitalRead(kLedPin));
  }
  cmd.parse_command();
  
  // 루프 실행 시간 측정 끝
    unsigned long loop_end = micros();
  
  // // 루프 실행 시간 출력
  //   Serial.print("Loop Execution Time (us): ");
  //   Serial.println(loop_end - loop_start);

}

// Sample the IMU, compute current commands and send to the ODrive.
void MotionController() {
  // Sample the IMU.
  imu::Vector<3> euler_angles = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  imu::Vector<3> gyro_rates = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);

  /* Motor controller */
  float pitch = (imu_enabled * euler_angles.y() + 1.3);
  // pitch 값을 바로 출력
    Serial.print("Pitch: ");
    Serial.println(pitch);
  float pitch_rate = imu_enabled * gyro_rates.y();
  float yaw_rate = imu_enabled * gyro_rates.z();

  // 각가속도 계산 (\(\ddot{\theta}\))
  float pitch_acceleration = (pitch_rate - prev_pitch_rate);
  prev_pitch_rate = pitch_rate;

  // 관성 토크 계산 (부호 음수 적용)
  float inertia_torque = -inertia * pitch_acceleration;

  // int throttle = kThrottlePolarity * rc_enabled * (throttle_pwm - kThrottlePwmOffset);
  int steering = kSteeringPolarity * rc_enabled * (steering_pwm - kSteeringPwmOffset);

  // Update the tilt safety flag based on the latest IMU reading.
  if (abs(euler_angles.y()) > kTiltDisengageThresholdDegrees) {
    tilt_limit_exceeded = true;
  }
  else {
    tilt_limit_exceeded = false;
  }

  // 목표 속도 및 현재 속도 계산
  if (throttle_pwm > 1492) {
    target_speed = (throttle_pwm - 1488) / 512.0;
  } else if (throttle_pwm < 1492) {
    target_speed = (throttle_pwm - 1488) / 488.0;
  } else {
    target_speed = 0;
  }

  float current_speed_left = odrive.GetVelocity(0) * kMotorDir0;
  float current_speed_right = odrive.GetVelocity(1) * kMotorDir1;

  // 비율 조정 변수 계산 (앞쪽으로 기울었을 때만 증가)
  float mgl = 0; // 기본값
  float pitch_diff_radians = radians(pitch - target_pitch);
  if (pitch - target_pitch > 0) {
    mgl = kpmgl * abs(sin(pitch_diff_radians));
  } else {
    mgl = -kpmgl * abs(sin(pitch_diff_radians));
  }

  float speed_error_left = target_speed - current_speed_left;
  float speed_error_right = target_speed - current_speed_right;

//   // 속도 관련 값 출력
// Serial.print("Target Speed: ");
// Serial.print(target_speed);
// Serial.print(" | Current Speed Left: ");
// Serial.print(current_speed_left);
// Serial.print(" | Current Speed Right: ");
// Serial.print(current_speed_right);
// Serial.print(" | ");
// Serial.print(speed_error_left);
// Serial.print(" | ");
// Serial.println(speed_error_right);

  float target_speed_torque = speed_p_gain * (speed_error_left + speed_error_right) / 2;

  target_speed_torque = constrain(target_speed_torque, -speed_p_gain, speed_p_gain);

  // 목표 speed_torque에 점진적으로 접근
  float diff = abs(target_speed_torque - speed_torque);
  float adjusted_speed_change = min(max_speed_change, diff);
  if (speed_torque < target_speed_torque) {
    speed_torque += adjusted_speed_change;
  } else if (speed_torque > target_speed_torque) {
    speed_torque -= adjusted_speed_change;
  }

  speed_torque = constrain(speed_torque, -speed_p_gain, speed_p_gain);

  // Serial.print("Pitch: ");
  // Serial.print(pitch);
  // Serial.print(" | Target Pitch: ");
  // Serial.print(target_pitch);
  // Serial.print(" | ");
  // Serial.println(speed_torque);

  //I 제어 항 계산
  if ((pitch - target_pitch > 0 && integralError < 0) || (pitch - target_pitch < 0 && integralError > 0)) {
    integralError = 0; // 완전히 초기화
  }

  if (pitch - target_pitch > 0) {
    integralError += kiBalance * abs(pitch_diff_radians); // 양수로 누적
  } else {
    integralError -= kiBalance * abs(pitch_diff_radians); // 음수로 누적
  }
  integralError = constrain(integralError, -maxIntegral, maxIntegral); // I 항 누적 제한

  // Balance controller.
  float pitch_radians = radians(pitch);
  float balance_controller = (sin(pitch_radians) * kpBalance + pitch_rate * kdBalance/100 + integralError + mgl);

  float steering_controller = (kpSteer * steering + yaw_rate * kdSteer);

  // 데드밴드 내에서 스케일링 팩터 계산
  if (abs(pitch - target_pitch) < deadband_threshold) {
      scale_factor = abs(pitch - target_pitch) / deadband_threshold; // 1~0으로 스케일링
  } else {
      scale_factor = 1.0; // 데드밴드 밖에서는 원래 출력 유지
  }

  target_command_right = (balance_controller - steering_controller - speed_torque + inertia_torque) * scale_factor;
  target_command_left = (balance_controller + steering_controller - speed_torque + inertia_torque) * scale_factor;

  // target_command_right와 target_command_left의 부호에 따라 initial_balance 조정
  target_command_right += (target_command_right != 0) ? ((target_command_right > 0) ? initial_balance / 100 : -initial_balance / 100) : 0;
  target_command_left += (target_command_left != 0) ? ((target_command_left > 0) ? initial_balance / 100 : -initial_balance / 100) : 0;

  target_command_right = constrain(target_command_right, -1 * kMaxAbsCurrent, kMaxAbsCurrent);
  target_command_left = constrain(target_command_left, -1 * kMaxAbsCurrent, kMaxAbsCurrent);

  odrive.SetCurrent(0, kMotorDir0 * target_command_right);
  odrive.SetCurrent(1, kMotorDir1 * target_command_left);

  // 이전 명령 값 업데이트
  prev_command_right = target_command_right;
  prev_command_left = target_command_left;

}

int EulerToMicroseconds(float euler) {
  constexpr float kMicroSecondsPerHalfRotation = 1000.0;
  return int((euler / 180.0) * kMicroSecondsPerHalfRotation);
}

// Engage or disengage motors.
void EngageMotors(bool request_motors_active) {
  if (request_motors_active != motors_active) {
    switch (request_motors_active) {
      case true:
        Serial.println("Engaging motors.");
        odrive.run_state(0, AXIS_STATE_CLOSED_LOOP_CONTROL, false);
        odrive.run_state(1, AXIS_STATE_CLOSED_LOOP_CONTROL, false);
        break;
      case false:
        Serial.println("Disengaging motors.");
        odrive.run_state(0, AXIS_STATE_IDLE, false);
        odrive.run_state(1, AXIS_STATE_IDLE, false);
        break;
    }
    motors_active = request_motors_active;
  }
}

// Generic interrupt callback function for RC PWM decoding. Measures the high pulse duration in microseconds.
void PwmInterruptCallback(unsigned long &last_rise_time, int &pwm_us, int pwm_input_pin) {
  if (digitalRead(pwm_input_pin)) { // Signal went HIGH.
    last_rise_time = micros();
  }
  else { // Signal went LOW.
    pwm_us = micros() - last_rise_time;
  }
}

// Wrappers for the three interrupt callbacks to decode the throttle, steering and engage PWM inputs.
void ThrottlePwmCallbackWrapper() {
  PwmInterruptCallback(last_throttle_pwm_rise_time, throttle_pwm, kThrottlePwmInputPin);
}

void SteeringPwmCallbackWrapper() {
  PwmInterruptCallback(last_steering_pwm_rise_time, steering_pwm, kSteeringPwmInputPin);
}

void EngagePwmCallbackWrapper() {
  PwmInterruptCallback(last_engage_pwm_rise_time, engage_pwm, kEngagePwmInputPin);
}

void PrintParameter(String name, float value) {
  Serial.print(name);
  Serial.print(":\t");
  Serial.println(value);
}

void PrintControllerParameters(float foo, float bar) {
  PrintParameter("kpBalance", kpBalance);
  PrintParameter("kdBalance", kdBalance);
  PrintParameter("kpThrottle", kpThrottle);
  PrintParameter("kpSteer", kpSteer);
  PrintParameter("kdSteer", kdSteer);
}

void PrintRcSignals() {
  Serial.print(throttle_pwm);
  Serial.print('\t');
  Serial.print(steering_pwm);
  Serial.print('\t');
  Serial.print(engage_pwm);
  Serial.print('\t');
  // 모터 최종 토크 값 출력
  Serial.print("Right Torque: ");
  Serial.print(target_command_right, 2); // 소수점 두 자리까지 출력
  Serial.print('\t');
  Serial.print("Left Torque: ");
  Serial.print(target_command_left, 2); // 소수점 두 자리까지 출력
  Serial.print('\t');
  Serial.println(speed_torque);
}

void SetKpBalance(float value, float foo) {
  Serial.println("Setting kpBalance to " + String(value));
  kpBalance = value;
}

void SetKdBalance(float value, float foo) {
  Serial.println("Setting kdBalance to " + String(value));
  kdBalance = value;
}

void Setkpmgl(float value, float foo) {  //mgl 변수
  Serial.println("Setting kpmgl to " + String(value));
  kpmgl = value;
}

void SetTargetPitch(float value, float foo) {   // 기울어진 무게중심 각도
    Serial.println("Setting target pitch to " + String(value));
    target_pitch = value;
}

void SetInitialBalance(float value, float foo) { // 명령어로 initial_balance 설정
  Serial.println("Setting initial_balance to " + String(value));
  initial_balance = value;
}

void SetKiBalance(float value, float foo) {
  Serial.println("Setting kiBalance to " + String(value));
  kiBalance = value; // 입력받은 값으로 I 게인 설정
}

// maxIntegral을 동적으로 설정하는 함수
void SetMaxIntegral(float value, float foo) {
  Serial.println("Setting maxIntegral to " + String(value));
  maxIntegral = value; // 입력받은 값으로 maxIntegral 설정
}

// 관성 모멘트를 설정하는 함수
void SetInertia(float value, float foo) {
    Serial.println("Setting inertia to " + String(value));
    inertia = value; // 사용자 입력값으로 inertia 업데이트
}

void EnableImu(float foo, float bar) {
  if (imu_enabled) {
    Serial.println("Disabling IMU.");
    imu_enabled = false;
  } else {
    Serial.println("Enabling IMU.");
    imu_enabled = true;
  }
}

void EnableRcControl(float foo, float bar) {
  if (rc_enabled) {
    Serial.println("Disabling RC control.");
    rc_enabled = false;
  } else {
    Serial.println("Enabling RC control.");
    rc_enabled = true;
  }
}

void EnableMotionController(float foo, float bar) {
  if (motion_controller_enabled) {
    Serial.println("Disabling motion controller.");
    motion_controller_enabled = false;
    odrive.SetCurrent(0, 0);
    odrive.SetCurrent(1, 0);
  } else {
    Serial.println("Enabling motion controller.");
    motion_controller_enabled = true;
  }
}

void EnableRcPrint(float foo, float bar) {
  if (rc_print_enabled) {
    Serial.println("Disabling PWM printout.");
    rc_print_enabled = false;
  } else {
    Serial.println("Enabling PWM printout.");
    rc_print_enabled = true;
  }
}
