// ODrive + RC receiver integration test.
// Adapted for this robot from the HoverBot odrive_test workflow:
// https://github.com/LuSeKa/HoverBot/tree/master/tests/odrive_test

#include <ODriveArduino.h>

namespace {
constexpr int kSteeringPwmPin = 2;
constexpr int kThrottlePwmPin = 3;
constexpr int kEngagePwmPin = 19;
constexpr int kLedPin = 13;

constexpr unsigned long kPcBaudrate = 115200;
constexpr unsigned long kOdriveBaudrate = 115200;
constexpr unsigned long kControlIntervalMs = 10;
constexpr unsigned long kActivationIntervalMs = 50;
constexpr unsigned long kPrintIntervalMs = 100;
constexpr unsigned long kBlinkIntervalMs = 200;

constexpr int kPwmCenterUs = 1500;
constexpr int kEngageThresholdUs = 1500;
constexpr float kThrottleGain = 0.015;
constexpr float kSteeringGain = 0.010;
constexpr float kMaxAbsCurrentA = 6.0;
constexpr int kMotorDir0 = 1;
constexpr int kMotorDir1 = -1;

volatile unsigned long steering_rise_us = 0;
volatile unsigned long throttle_rise_us = 0;
volatile unsigned long engage_rise_us = 0;
volatile int steering_pwm_us = kPwmCenterUs;
volatile int throttle_pwm_us = kPwmCenterUs;
volatile int engage_pwm_us = 1000;

unsigned long last_control_ms = 0;
unsigned long last_activation_ms = 0;
unsigned long last_print_ms = 0;
unsigned long last_blink_ms = 0;
bool motors_active = false;

ODriveArduino odrive(Serial3);
}

void setup() {
  pinMode(kLedPin, OUTPUT);
  pinMode(kSteeringPwmPin, INPUT);
  pinMode(kThrottlePwmPin, INPUT);
  pinMode(kEngagePwmPin, INPUT);

  Serial.begin(kPcBaudrate);
  Serial3.begin(kOdriveBaudrate);

  attachInterrupt(digitalPinToInterrupt(kSteeringPwmPin), steeringPwmInterrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(kThrottlePwmPin), throttlePwmInterrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(kEngagePwmPin), engagePwmInterrupt, CHANGE);

  Serial.println("throttle_us\tsteering_us\tengage_us\tleft_a\tright_a\tactive");
}

void loop() {
  const unsigned long now = millis();

  if (now - last_activation_ms >= kActivationIntervalMs) {
    last_activation_ms = now;
    updateMotorActivation(snapshotEngagePwm() > kEngageThresholdUs);
  }

  if (now - last_control_ms >= kControlIntervalMs) {
    last_control_ms = now;
    updateMotorCurrent();
  }

  if (now - last_print_ms >= kPrintIntervalMs) {
    last_print_ms = now;
    printStatus();
  }

  if (now - last_blink_ms >= kBlinkIntervalMs) {
    last_blink_ms = now;
    digitalWrite(kLedPin, !digitalRead(kLedPin));
  }
}

void updateMotorActivation(bool requested_active) {
  if (requested_active == motors_active) {
    return;
  }

  if (requested_active) {
    odrive.run_state(0, AXIS_STATE_CLOSED_LOOP_CONTROL, false);
    odrive.run_state(1, AXIS_STATE_CLOSED_LOOP_CONTROL, false);
  } else {
    odrive.SetCurrent(0, 0.0);
    odrive.SetCurrent(1, 0.0);
    odrive.run_state(0, AXIS_STATE_IDLE, false);
    odrive.run_state(1, AXIS_STATE_IDLE, false);
  }
  motors_active = requested_active;
}

void updateMotorCurrent() {
  const int throttle = snapshotThrottlePwm() - kPwmCenterUs;
  const int steering = snapshotSteeringPwm() - kPwmCenterUs;

  const float forward_current = kThrottleGain * throttle;
  const float steering_current = kSteeringGain * steering;
  const float right_current = constrain(forward_current + steering_current,
                                        -kMaxAbsCurrentA,
                                        kMaxAbsCurrentA);
  const float left_current = constrain(forward_current - steering_current,
                                       -kMaxAbsCurrentA,
                                       kMaxAbsCurrentA);

  if (motors_active) {
    odrive.SetCurrent(0, kMotorDir0 * right_current);
    odrive.SetCurrent(1, kMotorDir1 * left_current);
  }
}

void printStatus() {
  const int throttle = snapshotThrottlePwm();
  const int steering = snapshotSteeringPwm();
  const int engage = snapshotEngagePwm();
  const float forward_current = kThrottleGain * (throttle - kPwmCenterUs);
  const float steering_current = kSteeringGain * (steering - kPwmCenterUs);

  Serial.print(throttle);
  Serial.print('\t');
  Serial.print(steering);
  Serial.print('\t');
  Serial.print(engage);
  Serial.print('\t');
  Serial.print(constrain(forward_current - steering_current, -kMaxAbsCurrentA, kMaxAbsCurrentA), 3);
  Serial.print('\t');
  Serial.print(constrain(forward_current + steering_current, -kMaxAbsCurrentA, kMaxAbsCurrentA), 3);
  Serial.print('\t');
  Serial.println(motors_active ? 1 : 0);
}

void steeringPwmInterrupt() {
  decodePwm(kSteeringPwmPin, steering_rise_us, steering_pwm_us);
}

void throttlePwmInterrupt() {
  decodePwm(kThrottlePwmPin, throttle_rise_us, throttle_pwm_us);
}

void engagePwmInterrupt() {
  decodePwm(kEngagePwmPin, engage_rise_us, engage_pwm_us);
}

void decodePwm(int pin, volatile unsigned long &rise_time_us, volatile int &pulse_width_us) {
  if (digitalRead(pin)) {
    rise_time_us = micros();
  } else {
    pulse_width_us = static_cast<int>(micros() - rise_time_us);
  }
}

int snapshotSteeringPwm() {
  noInterrupts();
  const int value = steering_pwm_us;
  interrupts();
  return value;
}

int snapshotThrottlePwm() {
  noInterrupts();
  const int value = throttle_pwm_us;
  interrupts();
  return value;
}

int snapshotEngagePwm() {
  noInterrupts();
  const int value = engage_pwm_us;
  interrupts();
  return value;
}
