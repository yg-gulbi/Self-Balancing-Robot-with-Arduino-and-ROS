// Hall sensor bring-up sketch for a 3-line BLDC hall harness.
// Written for this robot and based on the test approach used by:
// https://github.com/CharlestonRobotics/ChIMP/tree/master/tests/hall_test

namespace {
constexpr int kHallPins[] = {A0, A1, A2};
constexpr unsigned long kPrintIntervalMs = 50;

int hall_state[3] = {HIGH, HIGH, HIGH};
int last_hall_state[3] = {HIGH, HIGH, HIGH};
unsigned long transition_count = 0;
unsigned long illegal_state_count = 0;
unsigned long last_print_ms = 0;
}

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 3; ++i) {
    pinMode(kHallPins[i], INPUT_PULLUP);
    hall_state[i] = digitalRead(kHallPins[i]);
    last_hall_state[i] = hall_state[i];
  }

  Serial.println("hall_a\thall_b\thall_c\tstate\ttransitions\tillegal");
  Serial.println("Send 'r' over Serial to reset the counters.");
}

void loop() {
  readHallState();

  if (stateChanged()) {
    if (isIllegalHallState()) {
      ++illegal_state_count;
    } else {
      ++transition_count;
    }
    copyHallState();
  }

  if (Serial.available() && Serial.read() == 'r') {
    transition_count = 0;
    illegal_state_count = 0;
  }

  if (millis() - last_print_ms >= kPrintIntervalMs) {
    last_print_ms = millis();
    printHallState();
  }
}

void readHallState() {
  for (int i = 0; i < 3; ++i) {
    hall_state[i] = digitalRead(kHallPins[i]);
  }
}

bool stateChanged() {
  for (int i = 0; i < 3; ++i) {
    if (hall_state[i] != last_hall_state[i]) {
      return true;
    }
  }
  return false;
}

void copyHallState() {
  for (int i = 0; i < 3; ++i) {
    last_hall_state[i] = hall_state[i];
  }
}

bool isIllegalHallState() {
  const int sum = hall_state[0] + hall_state[1] + hall_state[2];
  return sum == 0 || sum == 3;
}

void printHallState() {
  Serial.print(hall_state[0]);
  Serial.print('\t');
  Serial.print(hall_state[1]);
  Serial.print('\t');
  Serial.print(hall_state[2]);
  Serial.print('\t');
  Serial.print(hall_state[0]);
  Serial.print(hall_state[1]);
  Serial.print(hall_state[2]);
  Serial.print('\t');
  Serial.print(transition_count);
  Serial.print('\t');
  Serial.println(illegal_state_count);
}
