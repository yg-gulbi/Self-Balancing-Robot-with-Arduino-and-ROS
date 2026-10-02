# Sudden motor acceleration: isolate the ODrive path

[한국어](../../ko/troubleshooting/major/02-odrive-runaway.md) | [All troubleshooting](../../troubleshooting.md)

> I investigated intermittent uncontrolled motion by separating RC input, hall feedback, command conversion, and direct motor requests. Existing notes report symptom disappearance after firmware downgrade; the repository retains the corresponding testers and state/current-limit implementation.

## RC noise did not explain every motor symptom

The ODrive path sometimes accelerated unexpectedly or failed to stop normally. RC spikes also existed, but they did not explain all motor behavior. Adjusting gains on the complete robot would mix input interpretation, hall feedback, controller response, and ODrive settings.

The investigation became **which paths can be checked independently**, rather than treating runaway as one undifferentiated robot failure.

## Four checks reduced the number of coupled variables

| Check | Inspectable output | Cause candidate being separated |
| --- | --- | --- |
| Hall-only test | A/B/C state, transition count, illegal-state count | Sensor connection/state signals |
| Receiver-only test | Throttle/steering/engage pulse widths | RC input itself |
| Receiver–ODrive test | PWM, left/right current requests, active state | Input-to-current conversion |
| Direct-current test | Per-axis requests, stop and IDLE requests | Motor command path without RC or IMU |

The hall tester reads A0/A1/A2 and classifies all-low or all-high states as illegal. Its output fields are:

```text
hall_a    hall_b    hall_c    state    transitions    illegal
```

Those counters provide an observation method, not proof of a particular measured illegal-state count. Existing notes describe continuing the investigation after hall checks did not identify the culprit.

## Removing input paths narrowed the question

The [direct-current tester](../../../firmware/testers/motor_current_test/motor_current_test.ino) uses neither RC nor IMU. Serial commands request current on one/both axes and distinguish zero-current requests from IDLE requests.

| Command | What the code does |
| --- | --- |
| `m <axis> <amps>` | Requests bounded current on one axis |
| `b <amps>` | Requests current on both axes |
| `s` | Requests 0 A on both axes |
| `d` | Requests 0 A, then IDLE on both axes |

That separation makes receiver-input faults and motor-path behavior independently testable. This repository reconstructs and organizes earlier work; today's tester files alone do not recover every historical test result.

The existing investigation notes report that **firmware downgrade removed the runaway symptom**. This narrowed suspicion toward firmware/hall-drive compatibility. Exact before/after versions and a firmware-level fault trace are absent, so the result is an empirical resolution rather than a fully established internal bug.

## Motor state and command bounds remained explicit

The final sketch retains `EngageMotors()` for axis-state requests and `ApplyMotorCommands()` for current-request bounds:

```cpp
float current_command_0 = constrain(torque_input_0, -kMaxAbsCurrent, kMaxAbsCurrent);
float current_command_1 = constrain(torque_input_1, -kMaxAbsCurrent, kMaxAbsCurrent);
```

With `kMaxAbsCurrent=8.0`, outgoing Arduino requests are bounded to +/-8 A. Engage and tilt conditions participate in requests to switch between `IDLE` and `CLOSED_LOOP_CONTROL`.

A 0 A request, an IDLE request, and power disconnection are different actions. These files show command bounds and state logic; they are not actual-current measurements or independent containment of every ODrive fault.

## Resolution and root cause became separate conclusions

Firmware change affected the symptom. Subsystem tests also left a method for investigating recurrence under fewer coupled variables.

The engineering story is not only “downgrade fixed it.” It is the process of distinguishing input, feedback, and command paths and defining what each check can establish.

## Evidence a reviewer can inspect

**Work demonstrated:** system fault isolation, focused tests, and separating symptom resolution from root-cause certainty.

- [Hall tester](../../../firmware/testers/hall_sensor_test/hall_sensor_test.ino): state and counters.
- [Receiver tester](../../../firmware/testers/receiver_pwm_test/receiver_pwm_test.ino): input pulse widths.
- [Receiver–ODrive tester](../../../firmware/testers/odrive_receiver_test/odrive_receiver_test.ino): PWM/current relationship.
- [Current tester](../../../firmware/testers/motor_current_test/motor_current_test.ino): requests without RC/IMU.
- [Physical controller](../../../firmware/physical_balance_controller/physical_balance_controller.ino): limits and state requests.

Downgrade outcome is supported by existing notes. Version records, repeated identical-condition tests, and ODrive error-state logs would strengthen the connection between firmware change and outcome.
