# ODrive runaway and uncontrolled motor behavior

[한국어](../../ko/troubleshooting/major/02-odrive-runaway.md) | [Troubleshooting index](../../troubleshooting.md)

**Priority:** Major · Reconstructed from existing documentation and code

## 1. Problem definition

Intermittent sudden acceleration or failure to stop made full-body balance testing unreliable. The symptom could originate in RC input, hall feedback, the Arduino command path, or ODrive configuration/firmware.

## 2. Candidate solutions

Isolate each path with the hall-state tester, receiver tester, receiver-to-ODrive integration tester, and direct-current tester. Evaluate firmware compatibility after narrowing the subsystem. Independently restrict actuator commands and activation states; controller safeguards and firmware diagnosis answer different questions.

## 3. Execution

The original troubleshooting notes report that hall checks did not identify the culprit and that firmware downgrade removed the runaway symptom. They do not provide the exact downgrade target, a reproducible version matrix, or a firmware-level trace. The final controller switches axes between `IDLE` and `CLOSED_LOOP_CONTROL`, gates activation using tilt/engage state, and clamps outgoing current requests to `+/-8 A`. These are Arduino command limits, not proof that every internal ODrive fault is contained.

## 4. Reinterpreting the experience

A change that removes a symptom is useful evidence, but does not establish the precise root cause. Describe this as an empirical resolution after firmware downgrade. Subsystem isolation made the result interpretable; supervisory limits reduced the consequences of invalid controller states.

## 5. Summary and verification

Outcome: symptom disappearance after downgrade is reported in the prior notes; safety gates and command clamps can be inspected in code. Review the four tester sketches, `EngageMotors()`, and `ApplyMotorCommands()`. Remaining verification would require recording before/after firmware versions, identical test conditions, recurrence counts, and ODrive error states.

### Evidence

- [Hall tester](../../../firmware/testers/hall_sensor_test/hall_sensor_test.ino)
- [Receiver tester](../../../firmware/testers/receiver_pwm_test/receiver_pwm_test.ino)
- [ODrive receiver tester](../../../firmware/testers/odrive_receiver_test/odrive_receiver_test.ino)
- [Current tester](../../../firmware/testers/motor_current_test/motor_current_test.ino)
- [Physical controller](../../../firmware/physical_balance_controller/physical_balance_controller.ino)
