// ODrive motor current bring-up sketch for bench testing both wheel motors.
// Written for this robot and based on the command style used by:
// https://github.com/CharlestonRobotics/ChIMP/tree/master/tests/motor_test

#include <ODriveArduino.h>

namespace {
constexpr unsigned long kPcBaudrate = 115200;
constexpr unsigned long kOdriveBaudrate = 115200;
constexpr float kMaxAbsCurrentA = 8.0;

ODriveArduino odrive(Serial3);
String command_line;
}

void setup() {
  Serial.begin(kPcBaudrate);
  Serial3.begin(kOdriveBaudrate);
  command_line.reserve(48);
  printHelp();
}

void loop() {
  while (Serial.available()) {
    const char incoming = Serial.read();
    if (incoming == '\n' || incoming == '\r') {
      if (command_line.length() > 0) {
        handleCommand(command_line);
        command_line = "";
      }
    } else {
      command_line += incoming;
    }
  }
}

void handleCommand(String line) {
  line.trim();
  const char command = line.charAt(0);

  if (command == 'e') {
    setMotorState(AXIS_STATE_CLOSED_LOOP_CONTROL);
  } else if (command == 'd') {
    stopMotors();
    setMotorState(AXIS_STATE_IDLE);
  } else if (command == 's') {
    stopMotors();
  } else if (command == 'm') {
    commandSingleMotor(line);
  } else if (command == 'b') {
    commandBothMotors(line);
  } else {
    printHelp();
  }
}

void commandSingleMotor(const String &line) {
  const int first_space = line.indexOf(' ');
  const int second_space = line.indexOf(' ', first_space + 1);
  if (first_space < 0 || second_space < 0) {
    Serial.println("Usage: m <axis 0|1> <amps>");
    return;
  }

  const int axis = line.substring(first_space + 1, second_space).toInt();
  const float amps = constrain(line.substring(second_space + 1).toFloat(),
                               -kMaxAbsCurrentA,
                               kMaxAbsCurrentA);
  if (axis != 0 && axis != 1) {
    Serial.println("Axis must be 0 or 1.");
    return;
  }

  Serial.print("axis=");
  Serial.print(axis);
  Serial.print(" current_a=");
  Serial.println(amps, 3);
  odrive.SetCurrent(axis, amps);
}

void commandBothMotors(const String &line) {
  const int first_space = line.indexOf(' ');
  if (first_space < 0) {
    Serial.println("Usage: b <amps>");
    return;
  }

  const float amps = constrain(line.substring(first_space + 1).toFloat(),
                               -kMaxAbsCurrentA,
                               kMaxAbsCurrentA);
  odrive.SetCurrent(0, amps);
  odrive.SetCurrent(1, amps);
  Serial.print("both current_a=");
  Serial.println(amps, 3);
}

void stopMotors() {
  odrive.SetCurrent(0, 0.0);
  odrive.SetCurrent(1, 0.0);
  Serial.println("current set to 0 A on both axes");
}

void setMotorState(int requested_state) {
  odrive.run_state(0, requested_state, false);
  odrive.run_state(1, requested_state, false);
  Serial.print("requested_state=");
  Serial.println(requested_state);
}

void printHelp() {
  Serial.println("ODrive motor current test");
  Serial.println("e            enable closed-loop control");
  Serial.println("d            disable both axes");
  Serial.println("s            set both currents to 0 A");
  Serial.println("m <0|1> <A>  command one motor current");
  Serial.println("b <A>        command both motor currents");
}
