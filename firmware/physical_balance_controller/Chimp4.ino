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
#include <EEPROM.h>

namespace {
// Hardware settings.
constexpr unsigned short kLedPin = 13;
constexpr unsigned short kSteeringPwmInputPin = 2;
constexpr unsigned short kThrottlePwmInputPin = 3;
constexpr unsigned short kEngagePwmInputPin = 19;

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
float K_theta = 24;
float Ki = 0;
float K_theta_dot = 1.7;
float imu_angle_offset = 3.5;  // IMU 각도 오프셋 값
float adjustment_value = 0;  // 원하는 값으로 설정
float scaling_factor = 10000;
constexpr float kIntegralMax = 100.0;
float kpThrottle = 0.003;
float kpSteer = 0.001;
float kdSteer = 0.008; 
float theta = 0;
float kpspeed = 1.2;  // P 게인 속도 0.3
float kispeed = 0.1; // I 게인 속도 0.08
float kdspeed = 7;  // D 게인 속도
float kIntegralspeedMax = 5;  // I 최대값
constexpr float controlspeed = 6.0; // 최대 컨트롤 출력
constexpr float maxspeed = 1.5;   // 앞뒤 속도
constexpr float maxSpeedDiff = 0.7; // 좌우 속도차이
float integral_error = 0.0;
static float prev_speed_error = 0.0;
static float integral_speed_error = 0.0;
static float derivative_speed_error = 0.0;

constexpr float kWheelRadius = 0.0825;
float phi_dot_counts_0 = 0.0;
float phi_dot_counts_1 = 0.0;
float filtered_phi_dot_0 = 0.0;
float filtered_phi_dot_1 = 0.0;

constexpr float kAlpha_1 = 0.4;  // 필터 강도 (0.0 ~ 1.0 사이, 낮을수록 더 부드러움)
constexpr float kAlpha_2 = 0.02;
constexpr float kAlpha_3 = 1;
float filtered_throttle_pwm = 0;
float filtered_steering_pwm = 0;
float filtered_engage_pwm = 1000;
constexpr uint8_t kTiltDisengageThresholdDegrees = 30; // 전원이 차단되는 각도 임계값
constexpr float kMaxAbsCurrent = 8.0;
// LQR 연산 결과 저장 변수
float torque_input_0 = 0.0;
float torque_input_1 = 0.0;

// Task scheduling settings.
constexpr unsigned int kBlinkIntervalMs = 200;
constexpr unsigned int kControllerIntervalMs = 0;
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
ODriveArduino odrive(Serial3);

// Instantiate Metros for task scheduling.
Metro led_metro = Metro(kBlinkIntervalMs);
Metro controller_metro = Metro(kControllerIntervalMs);
Metro activation_metro = Metro(kActivationIntervalMs);
Metro print_metro = Metro(kPrintIntervalMs);
Command_processor cmd;
}

#define HISTORY_SIZE 3  // 최근 저장할 데이터 개수

float encoder_history_0[HISTORY_SIZE] = {0};  // 모터 0 속도 기록
float encoder_history_1[HISTORY_SIZE] = {0};  // 모터 1 속도 기록
int history_index = 0;  // 순환 버퍼 인덱스

float filterEncoderData(float new_value, float *history) {
    float prev_value = history[(history_index - 1 + HISTORY_SIZE) % HISTORY_SIZE];  // 직전 값

    // ✅ 값이 100 초과하면 이전 값 반환
    if (abs(new_value) > 100) {
        // Serial.println("⚠️ 이상치 감지! 이전 값 유지");
        return prev_value;
    }

    // ✅ 정상적인 값이면 저장 후 반환
    history[history_index] = new_value;
    history_index = (history_index + 1) % HISTORY_SIZE;

    return new_value;
}

void saveCalibration() {
    uint8_t calibrationData[22]; 
    bno.getSensorOffsets(calibrationData);
    Serial.println("Saving calibration...");

    for (int i = 0; i < 22; i++) {
        EEPROM.write(i, calibrationData[i]); 
    }
}

void loadCalibration() {
    uint8_t calibrationData[22]; 
    for (int i = 0; i < 22; i++) {
        calibrationData[i] = EEPROM.read(i);
    }
    bno.setSensorOffsets(calibrationData);
    Serial.println("Calibration loaded.");
}

void checkIMUCalibration() {  // ✅ setup() 밖에서 함수 정의
    uint8_t system, gyro, accel, mag;
    bno.getCalibration(&system, &gyro, &accel, &mag);
    
    Serial.print("System: "); Serial.print(system);
    Serial.print(" | Gyro: "); Serial.print(gyro);
    Serial.print(" | Accel: "); Serial.print(accel);
    Serial.print(" | Mag: "); Serial.println(mag);

    if (mag < 3) {
        Serial.println("⚠️ 지자계 보정 필요! 센서를 8자(Figure-8) 형태로 회전하세요.");
    } else {
        Serial.println("✅ 지자계 보정 완료!");
    }
}

void clearEEPROM() {
    for (int i = 0; i < EEPROM.length(); i++) {
        EEPROM.write(i, 0xFF);  // 모든 주소를 0xFF(초기 상태)로 변경
    }
    Serial.println("⚠️ EEPROM 초기화 완료! 보정 데이확인터를 다시 저장하세요.");
}

void setup() {
  // clearEEPROM();
  pinMode(kLedPin, OUTPUT);
  Serial.begin(kSerialBaudratePc);
  Serial3.begin(kSerialBaudrateOdrive);
  bno.setExtCrystalUse(true);
  if (!bno.begin()) {
    Serial.println("No BNO055 detected. Halting for safety.");
    while (true);
  }
  delay(1000);

  // loadCalibration();  // ✅ EEPROM에서 보정 데이터 불러오기
  // checkIMUCalibration(); // ✅ 센서 보정 상태 확인

  // PWM decoder interrupt handling.
  attachInterrupt(digitalPinToInterrupt(kSteeringPwmInputPin), SteeringPwmCallbackWrapper, CHANGE);
  attachInterrupt(digitalPinToInterrupt(kThrottlePwmInputPin), ThrottlePwmCallbackWrapper, CHANGE);
  attachInterrupt(digitalPinToInterrupt(kEngagePwmInputPin), EngagePwmCallbackWrapper, CHANGE);

  // Add commands to adjust parameters
  cmd.add_command('q', &SetK_theta, 1, "Set K_theta gain.");
  cmd.add_command('w', &SetKi, 1, "Set Ki gain.");
  cmd.add_command('e', &SetK_theta_dot, 1, "Set K_theta_dot gain.");
  cmd.add_command('r', &SetImuAngleOffset, 1, "Set IMU angle offset.");
  cmd.add_command('t', &SetAdjustmentValue, 1, "Set adjustment value.");
  cmd.add_command('y', &SetScalingFactor, 1, "Set tanh scaling factor.");
  cmd.add_command('a', &SetKpSpeed, 1, "Set kpspeed.");
  cmd.add_command('s', &SetKiSpeed, 1, "Set kispeed.");
  cmd.add_command('d', &SetKdSpeed, 1, "Set kdspeed.");
}

void loop() {

  if (controller_metro.check()) {
    // loop_start_time = micros();
    // 저역통과 필터 적용
  filtered_engage_pwm = kAlpha_2 * engage_pwm + (1.0 - kAlpha_2) * filtered_engage_pwm;
  filtered_throttle_pwm = kAlpha_1 * throttle_pwm + (1.0 - kAlpha_1) * filtered_throttle_pwm;
  filtered_steering_pwm = kAlpha_1 * steering_pwm + (1.0 - kAlpha_1) * filtered_steering_pwm;
    if (motion_controller_enabled) {
      LqrController();
      // 루프 끝에서 시간 차이 계산 및 출력

      ApplyMotorCommands();  // ✅ 모터에 최종 명령 보내기
  //      loop_duration = micros() - loop_start_time;
  // Serial.print("Loop duration: ");
  // Serial.print(loop_duration);
  // Serial.println(" us");
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
    engage_signal_persistence = constrain(engage_signal_persistence, -3, 3);
    engage = engage_signal_persistence > 0 ? true : false;
    bool request_motor_activation = engage && !tilt_limit_exceeded;

    EngageMotors(request_motor_activation);

    if (engage && !motors_active) {  
    // Serial.println("Controller engaged. Motors activated.");  // 전원 켜짐 메시지
    } 
    if (!engage && motors_active) {
    // Serial.println("Controller disengaged. Motors deactivated.");  // 전원 꺼짐 메시지
    initialized = false;
    }
    motors_active = engage && !tilt_limit_exceeded;
  }
  // if (led_metro.check()) {
  //   digitalWrite(kLedPin, !digitalRead(kLedPin));
  // }
  cmd.parse_command();
}

// LQR Controller Implementation
void LqrController() {

      // === 1. 버퍼 초기화 ===


  imu::Vector<3> euler_angles = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  imu::Vector<3> gyro_rates = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);

  // Update the tilt safety flag based on the latest IMU reading.
  if (abs(euler_angles.y() + imu_angle_offset) > kTiltDisengageThresholdDegrees) {
    tilt_limit_exceeded = true;
    return;
  } else {
    tilt_limit_exceeded = false;
  }

  // Body tilt angle and angular velocity
  theta = imu_enabled * (euler_angles.y() + imu_angle_offset);
  float theta_dot = imu_enabled * (gyro_rates.y());
  float yaw_rate = imu_enabled * gyro_rates.z();

  while (Serial3.available()) Serial3.read(); //엔코더 읽기 전 후 버퍼 초기화
  ///////////////////////////////////////////////////////////////////////

  // === 1. ODrive에서 엔코더 속도 읽기 ===
Serial3.print("j\n"
              "k\n"
              "k\n");

  phi_dot_counts_0 = Serial3.parseFloat();
  phi_dot_counts_1 = Serial3.parseFloat();

  while (Serial3.available()) Serial3.read(); //엔코더 읽기 전 후 버퍼 초기화

  filtered_phi_dot_0 = kAlpha_3 * phi_dot_counts_0 + (1.0 - kAlpha_3) * filtered_phi_dot_0;
  filtered_phi_dot_1 = kAlpha_3 * phi_dot_counts_1 + (1.0 - kAlpha_3) * filtered_phi_dot_1;

// === 2. 실제 선속도 계산 ===
float v_0 = kMotorDir0 * filtered_phi_dot_0 * 2 * PI * kWheelRadius;
float v_1 = kMotorDir1 * filtered_phi_dot_1 * 2 * PI * kWheelRadius;

int steering = kSteeringPolarity * rc_enabled * (filtered_steering_pwm - kSteeringPwmOffset);

// === 3. 컨트롤러 입력 변환 (-1 ~ 1) ===
float throttle_input = (filtered_throttle_pwm - 1491) / ((filtered_throttle_pwm > 1491) ? (1996 - 1491) : (1491 - 980));
float steering_input = (filtered_steering_pwm - 1492) / ((filtered_steering_pwm > 1492) ? (1999 - 1492) : (1492 - 977));

 if (abs(filtered_throttle_pwm - kThrottlePwmOffset) < kControlThreshold) {
    throttle_input = 0;
  }
  if (abs(filtered_steering_pwm - kSteeringPwmOffset) < kControlThreshold) {
    steering = 0;
  }

// === 4. 목표 속도 및 속도 차이 설정 ===
float target_speed = throttle_input * maxspeed;  // 최대 속도 2m/s

// === 5. 속도 제어 (기울기 조정) ===
float speed_error = target_speed - ((v_0 + v_1) / 2.0);
integral_speed_error += speed_error;
integral_speed_error = constrain(integral_speed_error, -kIntegralspeedMax, kIntegralspeedMax);
derivative_speed_error = speed_error - prev_speed_error;
prev_speed_error = speed_error;
float speed_control = kpspeed * speed_error + kispeed * integral_speed_error - kdspeed * derivative_speed_error;
speed_control = constrain(speed_control, -controlspeed, controlspeed);  //

// === 6. 조향 제어 (좌우 속도 보정) ===
float steering_controller = kpSteer * steering + kdSteer * yaw_rate;

  ///////////////////////////////////////////////////////////////////////////////////////

  // === 주행 중 정지 시 초기화 ===
  bool previous_was_driving = was_driving;  // 이전 상태 저장

  if (abs(filtered_throttle_pwm - kThrottlePwmOffset) > kControlThreshold ||
      abs(filtered_steering_pwm - kSteeringPwmOffset) > kControlThreshold) {
    ++drive_signal_persistence;
  } else {
    --drive_signal_persistence;
  }

  // 지속성 필터로 주행 상태 판별
  drive_signal_persistence = constrain(drive_signal_persistence, -2, 2);
  was_driving = drive_signal_persistence > 0 ? true : false;

  if (!motors_active || tilt_limit_exceeded) {
    integral_error = 0.0;  // ✅ 상태에 따라 즉시 초기화
} else {
    integral_error += theta;  // ✅ 초기화된 후에만 누적
    integral_error = constrain(integral_error, -kIntegralMax, kIntegralMax);
}

  float balance_controller = K_theta/100 * theta - K_theta_dot/100 * theta_dot + Ki/100 * integral_error;

  // 기존 토크 입력 계산
  torque_input_0 = (balance_controller - speed_control - steering_controller);
  torque_input_1 = (balance_controller - speed_control + steering_controller);

  // 양수일 때 일정값 추가, 음수일 때 일정값 감소
  torque_input_0 += adjustment_value * tanh(scaling_factor * filtered_phi_dot_0);;
  torque_input_1 -= adjustment_value * tanh(scaling_factor * filtered_phi_dot_1);

  // === 5. 수신된 값 출력 (명칭 없이 값만 출력) ===
  // Serial.print(filtered_phi_dot_0); Serial.print(" ");
  // Serial.print(filtered_phi_dot_1); Serial.println(" ");
  // Serial.print(phi_dot_counts_0); Serial.print(" ");
  // Serial.print(phi_dot_counts_1); Serial.println(" ");
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
  Serial.print(throttle_pwm);
  Serial.print('\t');
  Serial.print(steering_pwm);
  Serial.print('\t');
  Serial.print(filtered_engage_pwm);
  Serial.print('\t');
  Serial.print(engage_pwm);
  Serial.println();
}

// Command functions

void SetK_theta(float value, float foo) {
  Serial.println("Setting K_theta to " + String(value));
  K_theta = value;
}

void SetKi(float value, float foo) {
    Serial.println("Setting Ki to " + String(value));
    Ki = value;
}

void SetK_theta_dot(float value, float foo) {
  Serial.println("Setting K_theta_dot to " + String(value));
  K_theta_dot = value;
}

void SetImuAngleOffset(float value, float foo) {
  Serial.println("Setting IMU angle offset to " + String(value));
  imu_angle_offset = value;
}

void SetAdjustmentValue(float value, float foo) {
  Serial.println("Setting adjustment value to " + String(value));
  adjustment_value = value;
}

void SetScalingFactor(float value, float foo) {
  Serial.println("Setting tanh scaling factor to " + String(value));
  scaling_factor = value;
}

void SetKpSpeed(float value, float foo) {
  Serial.println("Setting kpspeed to " + String(value));
  kpspeed = value;
}

void SetKiSpeed(float value, float foo) {
  Serial.println("Setting kispeed to " + String(value));
  kispeed = value;
}

void SetKdSpeed(float value, float foo) {
  Serial.println("Setting kdspeed to " + String(value));
  kdspeed = value;
}

void EngageMotors(bool request_motors_active) {
  if (request_motors_active != motors_active) {  
    switch (request_motors_active) {
      case true:
        // Serial.println("Engaging motors.");
        odrive.run_state(0, AXIS_STATE_CLOSED_LOOP_CONTROL, false);
        odrive.run_state(1, AXIS_STATE_CLOSED_LOOP_CONTROL, false);
        break;
      case false:
        // Serial.println("Disengaging motors.");
        odrive.run_state(0, AXIS_STATE_IDLE, false);
        odrive.run_state(1, AXIS_STATE_IDLE, false);
        break;
    }
    motors_active = request_motors_active;
  }
}

void ApplyMotorCommands() {  
        // ✅ 모터 전류값 제한
    float current_command_0 = constrain(torque_input_0, -kMaxAbsCurrent, kMaxAbsCurrent);
    float current_command_1 = constrain(torque_input_1, -kMaxAbsCurrent, kMaxAbsCurrent);
    
    // ✅ ODrive 모터에 최종 명령 적용
    odrive.SetCurrent(0, kMotorDir0 * current_command_0);
    odrive.SetCurrent(1, kMotorDir1 * current_command_1);

    // Serial.print(current_command_0); Serial.print(" ");
    // Serial.println(current_command_1);
}
