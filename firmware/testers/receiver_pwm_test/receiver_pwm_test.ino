// RC receiver PWM inspection sketch.
// Written for this robot and based on the HoverBot receiver_test workflow:
// https://github.com/LuSeKa/HoverBot/tree/master/tests/receiver_test

namespace {
constexpr int kSteeringPwmPin = 2;
constexpr int kThrottlePwmPin = 3;
constexpr int kEngagePwmPin = 19;
constexpr int kLedPin = 13;

constexpr unsigned long kPcBaudrate = 115200;
constexpr unsigned long kPrintIntervalMs = 20;
constexpr unsigned long kBlinkIntervalMs = 200;

volatile unsigned long steering_rise_us = 0;
volatile unsigned long throttle_rise_us = 0;
volatile unsigned long engage_rise_us = 0;
volatile int steering_pwm_us = 1500;
volatile int throttle_pwm_us = 1500;
volatile int engage_pwm_us = 1000;

unsigned long last_print_ms = 0;
unsigned long last_blink_ms = 0;
}

void setup() {
  pinMode(kLedPin, OUTPUT);
  pinMode(kSteeringPwmPin, INPUT);
  pinMode(kThrottlePwmPin, INPUT);
  pinMode(kEngagePwmPin, INPUT);

  Serial.begin(kPcBaudrate);
  attachInterrupt(digitalPinToInterrupt(kSteeringPwmPin), steeringPwmInterrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(kThrottlePwmPin), throttlePwmInterrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(kEngagePwmPin), engagePwmInterrupt, CHANGE);

  Serial.println("throttle_us\tsteering_us\tengage_us");
}

void loop() {
  const unsigned long now = millis();

  if (now - last_print_ms >= kPrintIntervalMs) {
    last_print_ms = now;
    Serial.print(snapshotThrottlePwm());
    Serial.print('\t');
    Serial.print(snapshotSteeringPwm());
    Serial.print('\t');
    Serial.println(snapshotEngagePwm());
  }

  if (now - last_blink_ms >= kBlinkIntervalMs) {
    last_blink_ms = now;
    digitalWrite(kLedPin, !digitalRead(kLedPin));
  }
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
