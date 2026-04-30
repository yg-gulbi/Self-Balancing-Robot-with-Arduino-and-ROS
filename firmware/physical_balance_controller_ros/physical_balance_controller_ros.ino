// Physical self-balancing robot controller with rosserial state publishing.
//
// This sketch is based on firmware/physical_balance_controller/physical_balance_controller.ino
// and adds the ROS communication pattern preserved in archive/arduino_firmware/legacy_balance_controller.ino.
//
// Runtime structure:
// - Arduino keeps the real-time balance, speed, steering, safety, and ODrive current-control loop local.
// - rosserial publishes robot state to ROS as /imu and /odom for visualization, logging, SLAM, or navigation integration.
// - Serial is used by rosserial, so avoid Serial.print debugging while this sketch is connected to ROS.

#include <Wire.h>
#include <Metro.h>
#include <Servo.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <ODriveArduino.h>
#include <Command_processor.h>
#include <EEPROM.h>

#include <ros.h>
#include <sensor_msgs/Imu.h>
#include <nav_msgs/Odometry.h>

namespace {
// Hardware settings.
constexpr unsigned short kLedPin = 13;
constexpr unsigned short kSteeringPwmInputPin = 2;
constexpr unsigned short kThrottlePwmInputPin = 3;
constexpr unsigned short kEngagePwmInputPin = 19;

constexpr unsigned long kSerialBaudratePc = 115200;
constexpr unsigned long kSerialBaudrateOdrive = 115200;
constexpr int kMotorDir0 = 1;      // Can only be 1 or -1.
constexpr int kMotorDir1 = -1;     // Can only be 1 or -1.
constexpr int kSteeringPolarity = 1;
constexpr int kThrottlePolarity = 1;

// RC settings.
constexpr int kSteeringPwmOffset = 1492;
constexpr int kThrottlePwmOffset = 1488;
constexpr int kEngageThresholdPwm = 1500;
constexpr int kControlThreshold = 50;

// Balance, speed, and steering control parameters.
float K_theta = 24;
float Ki = 0;
float K_theta_dot = 1.7;
float imu_angle_offset = 3.5;
float adjustment_value = 0;
float scaling_factor = 10000;
constexpr float kIntegralMax = 100.0;
float kpThrottle = 0.003;
float kpSteer = 0.001;
float kdSteer = 0.008;
float theta = 0;
float kpspeed = 1.2;
float kispeed = 0.1;
float kdspeed = 7;
float kIntegralspeedMax = 5;
constexpr float controlspeed = 6.0;
constexpr float maxspeed = 1.5;
constexpr float maxSpeedDiff = 0.7;
float integral_error = 0.0;
float prev_speed_error = 0.0;
float integral_speed_error = 0.0;
float derivative_speed_error = 0.0;

constexpr float kWheelRadius = 0.0825;
constexpr float kWheelBase = 0.465;
float phi_dot_counts_0 = 0.0;
float phi_dot_counts_1 = 0.0;
float filtered_phi_dot_0 = 0.0;
float filtered_phi_dot_1 = 0.0;

constexpr float kAlpha_1 = 0.4;
constexpr float kAlpha_2 = 0.02;
constexpr float kAlpha_3 = 1.0;
float filtered_throttle_pwm = 0;
float filtered_steering_pwm = 0;
float filtered_engage_pwm = 1000;
constexpr uint8_t kTiltDisengageThresholdDegrees = 30;
constexpr float kMaxAbsCurrent = 8.0;
float torque_input_0 = 0.0;
float torque_input_1 = 0.0;

// Task scheduling settings.
constexpr unsigned int kBlinkIntervalMs = 200;
constexpr unsigned int kControllerIntervalMs = 0;
constexpr unsigned int kActivationIntervalMs = 50;
constexpr unsigned int kPrintIntervalMs = 50;
constexpr unsigned int kRosPublishIntervalMs = 50;

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
int drive_signal_persistence = 0;

// Interactive flags.
bool imu_enabled = true;
bool rc_enabled = true;
bool rc_print_enabled = false;
bool motion_controller_enabled = true;
constexpr bool kSerialGainTuningEnabled = false;

// ROS state.
ros::NodeHandle nh;
sensor_msgs::Imu imu_msg;
nav_msgs::Odometry odom_msg;
ros::Publisher imu_pub("/imu", &imu_msg);
ros::Publisher odom_pub("/odom", &odom_msg);

float odom_x = 0.0;
float odom_y = 0.0;
float odom_yaw = 0.0;
unsigned long last_odom_time_ms = 0;

Adafruit_BNO055 bno = Adafruit_BNO055();
ODriveArduino odrive(Serial3);

// Instantiate Metros for task scheduling.
Metro led_metro = Metro(kBlinkIntervalMs);
Metro controller_metro = Metro(kControllerIntervalMs);
Metro activation_metro = Metro(kActivationIntervalMs);
Metro print_metro = Metro(kPrintIntervalMs);
Metro ros_metro = Metro(kRosPublishIntervalMs);
Command_processor cmd;
}

void saveCalibration() {
  uint8_t calibrationData[22];
  bno.getSensorOffsets(calibrationData);

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
}

void clearEEPROM() {
  for (int i = 0; i < EEPROM.length(); i++) {
    EEPROM.write(i, 0xFF);
  }
}

void setup() {
  pinMode(kLedPin, OUTPUT);

  // Serial is used by rosserial in this variant.
  Serial.begin(kSerialBaudratePc);
  Serial3.begin(kSerialBaudrateOdrive);

  bno.setExtCrystalUse(true);
  if (!bno.begin()) {
    while (true) {
      digitalWrite(kLedPin, !digitalRead(kLedPin));
      delay(100);
    }
  }
  delay(1000);

  nh.getHardware()->setBaud(kSerialBaudratePc);
  nh.initNode();
  nh.advertise(imu_pub);
  nh.advertise(odom_pub);

  // PWM decoder interrupt handling.
  attachInterrupt(digitalPinToInterrupt(kSteeringPwmInputPin), SteeringPwmCallbackWrapper, CHANGE);
  attachInterrupt(digitalPinToInterrupt(kThrottlePwmInputPin), ThrottlePwmCallbackWrapper, CHANGE);
  attachInterrupt(digitalPinToInterrupt(kEngagePwmInputPin), EngagePwmCallbackWrapper, CHANGE);

  // Serial gain tuning is kept from the base controller, but disabled by default
  // because rosserial also owns Serial.
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
    filtered_engage_pwm = kAlpha_2 * engage_pwm + (1.0 - kAlpha_2) * filtered_engage_pwm;
    filtered_throttle_pwm = kAlpha_1 * throttle_pwm + (1.0 - kAlpha_1) * filtered_throttle_pwm;
    filtered_steering_pwm = kAlpha_1 * steering_pwm + (1.0 - kAlpha_1) * filtered_steering_pwm;

    if (motion_controller_enabled) {
      LqrController();
      ApplyMotorCommands();
    }
  }

  if (ros_metro.check()) {
    PublishRosImuData();
    PublishRosOdomData();
  }
  nh.spinOnce();

  if (print_metro.check()) {
    if (rc_print_enabled) {
      PrintRcSignals();
    }
  }

  if (activation_metro.check()) {
    if (filtered_engage_pwm > kEngageThresholdPwm) {
      ++engage_signal_persistence;
    } else {
      --engage_signal_persistence;
    }
    engage_signal_persistence = constrain(engage_signal_persistence, -3, 3);
    engage = engage_signal_persistence > 0;

    bool request_motor_activation = engage && !tilt_limit_exceeded;
    EngageMotors(request_motor_activation);

    if (!engage && motors_active) {
      initialized = false;
    }
    motors_active = request_motor_activation;
  }

  if (led_metro.check()) {
    digitalWrite(kLedPin, !digitalRead(kLedPin));
  }

  if (kSerialGainTuningEnabled) {
    cmd.parse_command();
  }
}

void PublishRosImuData() {
  imu_msg.header.stamp = nh.now();
  imu_msg.header.frame_id = "base_link";

  imu::Quaternion quat = bno.getQuat();
  imu_msg.orientation.w = quat.w();
  imu_msg.orientation.x = quat.x();
  imu_msg.orientation.y = quat.y();
  imu_msg.orientation.z = quat.z();

  imu_msg.orientation_covariance[0] = 0.0025;
  imu_msg.orientation_covariance[1] = 0;
  imu_msg.orientation_covariance[2] = 0;
  imu_msg.orientation_covariance[3] = 0;
  imu_msg.orientation_covariance[4] = 0.0025;
  imu_msg.orientation_covariance[5] = 0;
  imu_msg.orientation_covariance[6] = 0;
  imu_msg.orientation_covariance[7] = 0;
  imu_msg.orientation_covariance[8] = 0.0025;

  imu::Vector<3> gyro_rates = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);
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

  imu::Vector<3> acceleration = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
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

  imu_pub.publish(&imu_msg);
}

void PublishRosOdomData() {
  unsigned long current_time_ms = millis();
  if (last_odom_time_ms == 0) {
    last_odom_time_ms = current_time_ms;
    return;
  }

  float dt = (current_time_ms - last_odom_time_ms) / 1000.0;
  last_odom_time_ms = current_time_ms;
  if (dt <= 0) {
    return;
  }

  float left_velocity = kMotorDir0 * filtered_phi_dot_0 * 2.0 * PI * kWheelRadius;
  float right_velocity = kMotorDir1 * filtered_phi_dot_1 * 2.0 * PI * kWheelRadius;
  float linear_velocity = (left_velocity + right_velocity) / 2.0;
  float angular_velocity = (right_velocity - left_velocity) / kWheelBase;

  odom_yaw += angular_velocity * dt;
  odom_x += linear_velocity * cos(odom_yaw) * dt;
  odom_y += linear_velocity * sin(odom_yaw) * dt;

  odom_msg.header.stamp = nh.now();
  odom_msg.header.frame_id = "odom";
  odom_msg.child_frame_id = "base_footprint";

  odom_msg.pose.pose.position.x = odom_x;
  odom_msg.pose.pose.position.y = odom_y;
  odom_msg.pose.pose.position.z = 0.0;
  odom_msg.pose.pose.orientation.w = cos(odom_yaw / 2.0);
  odom_msg.pose.pose.orientation.x = 0.0;
  odom_msg.pose.pose.orientation.y = 0.0;
  odom_msg.pose.pose.orientation.z = sin(odom_yaw / 2.0);

  for (int i = 0; i < 36; i++) {
    odom_msg.pose.covariance[i] = 0;
    odom_msg.twist.covariance[i] = 0;
  }
  odom_msg.pose.covariance[0] = 0.02;
  odom_msg.pose.covariance[7] = 0.02;
  odom_msg.pose.covariance[35] = 0.04;
  odom_msg.twist.covariance[0] = 0.02;
  odom_msg.twist.covariance[35] = 0.04;

  odom_msg.twist.twist.linear.x = linear_velocity;
  odom_msg.twist.twist.linear.y = 0.0;
  odom_msg.twist.twist.angular.z = angular_velocity;

  odom_pub.publish(&odom_msg);
}

// LQR-style balance controller with speed and steering correction.
void LqrController() {
  imu::Vector<3> euler_angles = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  imu::Vector<3> gyro_rates = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);

  if (abs(euler_angles.y() + imu_angle_offset) > kTiltDisengageThresholdDegrees) {
    tilt_limit_exceeded = true;
    return;
  }
  tilt_limit_exceeded = false;

  theta = imu_enabled * (euler_angles.y() + imu_angle_offset);
  float theta_dot = imu_enabled * gyro_rates.y();
  float yaw_rate = imu_enabled * gyro_rates.z();

  while (Serial3.available()) {
    Serial3.read();
  }

  Serial3.print("j\n"
                "k\n"
                "k\n");

  phi_dot_counts_0 = Serial3.parseFloat();
  phi_dot_counts_1 = Serial3.parseFloat();

  while (Serial3.available()) {
    Serial3.read();
  }

  filtered_phi_dot_0 = kAlpha_3 * phi_dot_counts_0 + (1.0 - kAlpha_3) * filtered_phi_dot_0;
  filtered_phi_dot_1 = kAlpha_3 * phi_dot_counts_1 + (1.0 - kAlpha_3) * filtered_phi_dot_1;

  float v_0 = kMotorDir0 * filtered_phi_dot_0 * 2.0 * PI * kWheelRadius;
  float v_1 = kMotorDir1 * filtered_phi_dot_1 * 2.0 * PI * kWheelRadius;

  int steering = kSteeringPolarity * rc_enabled * (filtered_steering_pwm - kSteeringPwmOffset);

  float throttle_input = (filtered_throttle_pwm - 1491) /
                         ((filtered_throttle_pwm > 1491) ? (1996 - 1491) : (1491 - 980));

  if (abs(filtered_throttle_pwm - kThrottlePwmOffset) < kControlThreshold) {
    throttle_input = 0;
  }
  if (abs(filtered_steering_pwm - kSteeringPwmOffset) < kControlThreshold) {
    steering = 0;
  }

  float target_speed = throttle_input * maxspeed;
  float speed_error = target_speed - ((v_0 + v_1) / 2.0);
  integral_speed_error += speed_error;
  integral_speed_error = constrain(integral_speed_error, -kIntegralspeedMax, kIntegralspeedMax);
  derivative_speed_error = speed_error - prev_speed_error;
  prev_speed_error = speed_error;
  float speed_control = kpspeed * speed_error + kispeed * integral_speed_error - kdspeed * derivative_speed_error;
  speed_control = constrain(speed_control, -controlspeed, controlspeed);

  float steering_controller = kpSteer * steering + kdSteer * yaw_rate;

  if (abs(filtered_throttle_pwm - kThrottlePwmOffset) > kControlThreshold ||
      abs(filtered_steering_pwm - kSteeringPwmOffset) > kControlThreshold) {
    ++drive_signal_persistence;
  } else {
    --drive_signal_persistence;
  }
  drive_signal_persistence = constrain(drive_signal_persistence, -2, 2);
  was_driving = drive_signal_persistence > 0;

  if (!motors_active || tilt_limit_exceeded) {
    integral_error = 0.0;
    integral_speed_error = 0.0;
    derivative_speed_error = 0.0;
  } else {
    integral_error += theta;
    integral_error = constrain(integral_error, -kIntegralMax, kIntegralMax);
  }

  float balance_controller = K_theta / 100.0 * theta -
                             K_theta_dot / 100.0 * theta_dot +
                             Ki / 100.0 * integral_error;

  torque_input_0 = balance_controller - speed_control - steering_controller;
  torque_input_1 = balance_controller - speed_control + steering_controller;

  torque_input_0 += adjustment_value * tanh(scaling_factor * filtered_phi_dot_0);
  torque_input_1 -= adjustment_value * tanh(scaling_factor * filtered_phi_dot_1);
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
  // Disabled by default because raw Serial output corrupts rosserial traffic.
  Serial.print(throttle_pwm);
  Serial.print('\t');
  Serial.print(steering_pwm);
  Serial.print('\t');
  Serial.print(filtered_engage_pwm);
  Serial.print('\t');
  Serial.print(engage_pwm);
  Serial.println();
}

void SetK_theta(float value, float foo) {
  K_theta = value;
}

void SetKi(float value, float foo) {
  Ki = value;
}

void SetK_theta_dot(float value, float foo) {
  K_theta_dot = value;
}

void SetImuAngleOffset(float value, float foo) {
  imu_angle_offset = value;
}

void SetAdjustmentValue(float value, float foo) {
  adjustment_value = value;
}

void SetScalingFactor(float value, float foo) {
  scaling_factor = value;
}

void SetKpSpeed(float value, float foo) {
  kpspeed = value;
}

void SetKiSpeed(float value, float foo) {
  kispeed = value;
}

void SetKdSpeed(float value, float foo) {
  kdspeed = value;
}

void EngageMotors(bool request_motors_active) {
  if (request_motors_active != motors_active) {
    switch (request_motors_active) {
      case true:
        odrive.run_state(0, AXIS_STATE_CLOSED_LOOP_CONTROL, false);
        odrive.run_state(1, AXIS_STATE_CLOSED_LOOP_CONTROL, false);
        break;
      case false:
        odrive.run_state(0, AXIS_STATE_IDLE, false);
        odrive.run_state(1, AXIS_STATE_IDLE, false);
        break;
    }
    motors_active = request_motors_active;
  }
}

void ApplyMotorCommands() {
  if (!motors_active || tilt_limit_exceeded) {
    odrive.SetCurrent(0, 0);
    odrive.SetCurrent(1, 0);
    return;
  }

  float current_command_0 = constrain(torque_input_0, -kMaxAbsCurrent, kMaxAbsCurrent);
  float current_command_1 = constrain(torque_input_1, -kMaxAbsCurrent, kMaxAbsCurrent);

  odrive.SetCurrent(0, kMotorDir0 * current_command_0);
  odrive.SetCurrent(1, kMotorDir1 * current_command_1);
}
