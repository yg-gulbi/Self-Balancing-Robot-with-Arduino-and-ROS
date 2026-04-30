# Project Troubleshooting Summary

This page is the engineering trouble log behind the project. It explains the hardest problems I ran into, how I split them into smaller checks, and what control or architecture changes I made in response. I tried to keep it specific where the code or notes are clear, and careful where the exact root cause was never proven perfectly.

## How To Read This Page

- `Confirmed`: directly supported by firmware, tester sketches, public docs, or repeatable project notes.
- `Likely inference`: the explanation that fits the project material best, even if I cannot prove it like a lab result.
- `Not claimed`: something I may have explored, but I am not presenting it as a finished final result.

## What This Summary Is Based On

This summary is based primarily on:

- `firmware/physical_balance_controller/physical_balance_controller.ino`
- `firmware/physical_balance_controller/control_algorithm.md`
- `firmware/testers/*`
- `archive/arduino_firmware/*`
- `ros_ws/src/*`
- `docs/*.md`

External references helped where it made sense, especially around ODrive hoverboard bring-up and RF or EMI behavior, but the main goal of this page is to explain what this specific project actually did in practice.

## High-Level Problem Map

| Area | Main symptom | How it was isolated | Main mitigation or design response | How sure I am |
| --- | --- | --- | --- | --- |
| RC receiver path | Wheel twitch and PWM spikes | Serial monitoring, wiring changes, metal proximity sensitivity, foil experiment | Better receiver placement, twisted signal-ground routing, filtering, deadband, engage persistence | Mostly confirmed, with some inference |
| ODrive path | Intermittent uncontrolled motor behavior | Hall test, receiver test, Arduino path checks, ODrive-focused tester sketches, firmware downgrade result | Firmware downgrade plus tighter safety gating in the controller | Mostly confirmed, with some inference |
| Physical balancing | Fall risk and extreme tuning sensitivity | Tethered practice, staged subsystem bring-up, gain iteration | LQR-style balance term, speed loop, steering loop, current clamp, tilt cutoff | Confirmed |
| IMU repeatability | Balance point drift and inconsistent upright reference | Calibration checks, saved offsets, angle offset tuning | BNO055 calibration handling and explicit `imu_angle_offset` | Confirmed |
| Navigation command path | High-level ROS commands could not directly drive a balancing robot | Simulation controller design and `/before_vel` pipeline | Separate navigation intent from final low-level balance output | Confirmed |

## Control Techniques Introduced To Solve Real Problems

The final controller is not just one control equation. It is a stack of countermeasures that came from real hardware failures and tuning lessons.

| Technique | Problem it addresses | Where it shows up in code |
| --- | --- | --- |
| Low-pass filtering on RC PWM | Short spikes and noisy command edges | `filtered_throttle_pwm`, `filtered_steering_pwm`, `filtered_engage_pwm`, `kAlpha_1`, `kAlpha_2` |
| Neutral offset and deadband | Receiver center drift causing unintended motion | `kSteeringPwmOffset`, `kThrottlePwmOffset`, `kControlThreshold` |
| Engage persistence | False motor enable or disable from noisy PWM | `engage_signal_persistence`, `EngageMotors()` |
| Tilt cutoff | Cutting motor command before the robot reaches a hard fall posture | `kTiltDisengageThresholdDegrees`, `tilt_limit_exceeded` |
| Current clamp | Limiting violent recovery or runaway commands | `kMaxAbsCurrent`, `ApplyMotorCommands()` |
| LQR-style balance term | Keeping the unstable body upright | `balance_controller` built from `theta`, `theta_dot`, `integral_error` |
| PID-like speed correction | Making forward and backward motion coexist with balancing | `speed_control`, `kpspeed`, `kispeed`, `kdspeed` |
| Steering plus yaw damping | Turning without destroying the common balance effort | `steering_controller`, `kpSteer`, `kdSteer` |
| Separate ROS intent topic | Preventing high-level navigation from bypassing balance logic | `/before_vel` design in `ros_ws/src/balance_robot_control` and `ros_ws/src/navigation` |

## 1A. Why Filtering Was Needed At All

One of the easiest mistakes in a balancing-robot project is to treat filtering as a cosmetic cleanup step. In this project, the filters existed because several signals were noisy enough to create control problems if they were trusted raw.

### The Underlying Problem

The controller depended on signals that were all vulnerable in different ways:

- RC PWM could jump briefly or drift around the neutral point.
- ODrive wheel-speed feedback could contain sudden spikes, parse irregularities, or implausible jumps.
- IMU-derived body motion was useful, but still had to coexist with calibration offset, mounting bias, and mechanical vibration.

That matters because this robot was not controlling a stable plant. A short-lived bad sample was not just a logging nuisance. It could directly alter the equilibrium point, torque request, or motor activation state of an unstable machine.

### What Would Go Wrong Without Filters

If the raw signals were used directly, the project would face several concrete failure modes:

- a short RC PWM spike could look like a real throttle or steering command
- a tiny center-value drift could make the robot creep or keep "asking" for motion near neutral
- a noisy engage channel could arm or disarm the motors at the wrong time
- a spiky wheel-speed estimate could distort the speed loop and produce an exaggerated corrective current
- a single bad encoder or ODrive read could be interpreted as a real state change instead of a measurement fault

This is why the project did not rely on one generic filter. Different failure modes got different countermeasures.

### What Was Introduced

For RC input:

- exponential low-pass filtering on `throttle`, `steering`, and `engage`
- neutral offsets instead of assuming exact `1500 us` center
- deadband around the neutral region
- persistence logic before the engage signal can change motor state

For wheel or ODrive feedback:

- repeated defensive parsing in older firmware
- median-style outlier replacement and adaptive threshold checks in archived firmware
- first-order smoothing of wheel-speed estimates in the cleaned controller
- constrained integrators so noisy speed feedback cannot accumulate forever

### Why This Is A Control Design Decision

The important point is that filtering here was part of the control architecture, not a post-processing detail. I ended up learning that `measurement trust` had to be designed explicitly. The controller only becomes meaningful after deciding which signals are trustworthy enough to affect balance, speed bias, steering, and motor activation.

## 1. RC Receiver PWM Spike And Wheel Twitch

### Symptom

During physical driving and balancing bring-up, the wheel could twitch even when I did not intend a sudden command change. I also checked through Arduino serial output that the FrSky X8R PWM readings could jump unexpectedly.

### What Was Checked

- Receiver PWM was inspected directly with `receiver_pwm_test.ino`.
- Wiring and receiver-side connections were reworked multiple times.
- The problem changed noticeably depending on receiver placement near metal structure.
- An aluminum-foil troubleshooting experiment was tried to test whether nearby conductive material was affecting the signal path.

### What It Most Likely Meant

The strongest explanation is a combination of RF placement sensitivity and ordinary signal-integrity problems on single-ended PWM lines:

- The receiver and antenna behavior became worse near conductive structure.
- The foil experiment made the spikes worse instead of better, which is consistent with detuning or disturbing the RF environment rather than fixing it.
- Long or loosely routed PWM lines are vulnerable to induced noise and ground-reference problems.

This is best described as a `receiver placement plus wiring integrity` problem, not just a software bug.

### What Was Changed

Hardware-side mitigation:

- Keep the receiver and antenna away from metal structure, power wiring, and high-current motor-control regions.
- Route each PWM signal with its ground return as closely as possible.
- Treat receiver placement as part of the control system, not as an afterthought.

Firmware-side mitigation:

- Apply low-pass filtering to throttle, steering, and engage PWM before use.
- Use neutral offsets rather than assuming every channel centers at exactly `1500 us`.
- Apply deadband so small PWM drift does not become a drive command.
- Require persistence before the engage signal can activate the motors.

### Why The Control Changes Matter

Without these changes, the balance controller would interpret a noisy RC pulse as real intent. On a normal wheeled robot that might cause a brief jerk. On a self-balancing robot it is worse: a false throttle or steering command changes the target equilibrium of an already unstable plant. That means RC filtering was not a comfort feature. It was part of making the closed loop trustworthy enough to stand up.

## 2. ODrive Runaway And Sudden Uncontrolled Motor Behavior

### Symptom

One of the most serious issues in the project was intermittent uncontrolled motor behavior through the ODrive path. In the user description, the robot could suddenly accelerate or fail to stop cleanly, which is exactly the kind of failure mode that makes balancing tests dangerous.

### Isolation Strategy

The project did not immediately blame the whole robot. It broke the problem into smaller paths:

- `hall_sensor_test.ino` checked hall-state transitions and illegal states.
- `receiver_pwm_test.ino` checked whether RC PWM itself looked sane.
- `odrive_receiver_test.ino` checked the integration path between receiver commands and ODrive behavior.
- `motor_current_test.ino` reduced the problem to direct current-command behavior.
- The final physical controller added explicit engage and safety state management instead of assuming ODrive should always stay active.

That isolation process is important because it shows the failure was treated as a system-debugging problem, not as random gain tuning.

### Most Honest Conclusion

The most defensible public statement is:

> The runaway issue was isolated toward the ODrive firmware or hoverboard hall-sensor bring-up path rather than being proven as a receiver-side or Arduino-side PWM problem.

Why I think that statement is fair:

- Hall testing looked normal enough to keep investigating elsewhere.
- Receiver PWM issues existed, but they did not fully explain the ODrive-specific symptom pattern.
- A known ODrive community pattern existed around hoverboard hall behavior after firmware `0.5.1`.
- Downgrading firmware removed the runaway symptom in this project.

The exact firmware-level root cause was not formally proven, so the most honest wording is `empirically solved by firmware downgrade`, not `fully root-caused firmware bug`.

### What Was Introduced In Response

Even after the downgrade, the controller architecture was tightened so that ODrive could not be treated as an always-trusting black box:

- `EngageMotors()` explicitly switches axes between `CLOSED_LOOP_CONTROL` and `IDLE`.
- Large tilt can force `tilt_limit_exceeded`, which blocks motor activation.
- Final current is clamped to `+/-8 A` before calling `ODrive.SetCurrent`.
- Integral terms are reset when motors are inactive or tilt safety is exceeded.

These are not cosmetic safety checks. They are supervisory control features added because the hardware path had already demonstrated that it could misbehave in ways a pure balance equation would not catch.

## 3. Why A Simple Controller Was Not Enough

The physical robot could not be stabilized by treating throttle as direct wheel speed and calling it done. A balancing robot is an unstable mechanical system. The controller has to keep the body upright first, then deliberately bias that upright condition to create motion, and then split wheel effort to steer.

The final firmware therefore behaves like a layered controller:

```text
RC intent + IMU state + wheel-speed feedback
  -> signal filtering and state gating
  -> balance correction
  -> speed correction
  -> steering correction
  -> left/right current mixing
  -> current clamp
  -> ODrive current commands
```

That structure is documented visually in [control_algorithm.md](../firmware/physical_balance_controller/control_algorithm.md).

## 4. The Control Algorithm Adopted To Solve The Real Robot Problems

### 4.1 Balance Term

The core stabilizing term in the final controller is:

```text
balance_controller =
  (K_theta / 100)     * theta
- (K_theta_dot / 100) * theta_dot
+ (Ki / 100)          * integral_error
```

Interpretation:

- `theta` is body tilt angle from the BNO055, adjusted by `imu_angle_offset`.
- `theta_dot` is body angular velocity from the gyroscope.
- `integral_error` accumulates tilt error only while the system is in an active safe state.

Why this matters:

- `theta` is the gravity problem. If the body leans, the wheels must move to get back under the center of mass.
- `theta_dot` is the damping problem. Without it, the robot may react in the right direction but overshoot hard.
- `integral_error` is the bias problem. Small mounting offsets, unequal friction, or slight sensor bias can otherwise keep the robot from settling around a practical upright point.

This is why the controller is reasonably described as `LQR-style` rather than just `PID`. It uses explicit state feedback on angle and angular velocity, then augments that with additional terms required by the real hardware.

### 4.2 Speed Loop

Forward and backward motion are not applied as raw wheel commands. They are turned into a speed target and then a correction term:

```text
target_speed = throttle_input * maxspeed
speed_error = target_speed - average_wheel_speed

speed_control =
  kpspeed * speed_error
+ kispeed * integral_speed_error
- kdspeed * derivative_speed_error
```

Why this was necessary:

- A balancing robot cannot simply "drive forward" the same way a four-wheel robot can.
- To move forward, it must intentionally allow a controlled forward bias while still staying inside the stabilizing region.
- The speed loop therefore shifts the operating point of the balance controller instead of replacing it.

This is one of the strongest engineering decisions in the repository. It shows the project did not confuse `motion request` with `actuator authority`.

### 4.3 Steering Loop

Steering is computed separately:

```text
steering_controller = kpSteer * steering + kdSteer * yaw_rate
```

Then the steering term is mixed with opposite signs:

```text
motor_0 = balance_controller - speed_control - steering_controller
motor_1 = balance_controller - speed_control + steering_controller
```

Why this structure matters:

- The balance term is a common-mode effort that both wheels need in the same direction.
- The steering term is a differential effort that should oppose between the two wheels.
- Mixing them this way lets the robot turn while preserving the shared stabilizing action.

This is a much better fit than trying to inject left and right wheel commands independently from the RC receiver.

### 4.4 Safety Supervisor

The controller is only credible because it also supervises when not to trust itself.

Key logic:

- `filtered_engage_pwm` must pass a persistence check before activation.
- `abs(theta)` above `kTiltDisengageThresholdDegrees` sets `tilt_limit_exceeded`.
- Motors can be pushed to `IDLE` instead of remaining in a control state forever.
- Final current is constrained by `kMaxAbsCurrent`.

The physical tilt cutoff was set to `30 deg` because the protection bracket touches the floor at about `28 deg`. That made the cutoff a practical damage-prevention threshold, not just an arbitrary control number.

In a balancing robot, these supervisory rules are part of the control design. They are not merely protective wrappers around the "real" algorithm. The real algorithm includes the conditions under which actuation is allowed.

## 5. Balance Tuning Strategy And Fall-Risk Management

### Practical Problem

Even a reasonable equation can fail badly if the gains are tuned on the full robot too early. A balancing robot has strong coupling among body angle, wheel speed, current response, mass distribution, and sensor bias. That means uncontrolled early tuning is more like crash testing than engineering.

### What The Project Did Instead

- Bring up subsystems separately before full-body balancing.
- Use bench tests for motors and hall feedback first.
- Use tethered practice before trusting untethered physical driving.
- Keep current limited during early tuning.
- Reset or suppress integral accumulation when the robot is not in a valid active state.

### Why This Is A Real Control Strategy

The tuning process itself was part of the solution. The project did not solve balancing by finding one magic gain set. It solved balancing by constraining the search space:

- no raw RC commands directly to motor current
- no full trust in noisy center values
- no unlimited current while experimenting
- no continued integration after a fall angle
- no assumption that one subsystem being healthy proves the full closed loop is healthy

That is the difference between a mathematically plausible controller and a controller that can survive the real bench-to-floor transition.

## 6. IMU Calibration And Upright Reference Management

### Problem

If the IMU does not agree with the robot's real upright posture, then the controller will try to stabilize around the wrong angle. On a balancing robot this can show up as creeping, persistent correction, asymmetric recovery, or immediate fall behavior.

### Evidence In The Firmware

- The code halts if the BNO055 is not detected.
- Calibration save and load helpers exist through EEPROM.
- Calibration status can be printed and checked.
- The controller uses an explicit `imu_angle_offset`.

### Interpretation

The project learned that `upright` is not a purely theoretical reference. It is a calibrated operating condition that depends on sensor alignment, mounting angle, and calibration state. That is why the controller includes both calibration handling and an explicit angle offset rather than assuming raw IMU values are directly usable.

## 7. Wheel-Speed Feedback Trust And Serial Robustness

The ODrive feedback path also shows the same defensive mindset:

- Legacy firmware used repeated reads, smoothing, and outlier handling around ODrive feedback.
- The final controller flushes `Serial3` before and after wheel-speed reads.
- Speed-related integrals are constrained so noisy feedback cannot accumulate unbounded correction.

This suggests the project recognized a key truth of balancing systems: feedback quality is part of controller stability. A mathematically correct control law can still fail if its velocity estimate is delayed, spiky, or parsed inconsistently.

### What Filtering Problem This Solved

The wheel-speed path was not filtered just to make plots look nicer. It was solving a control problem:

- the speed loop depends on average wheel velocity to decide how much forward or backward bias to add
- if that velocity estimate suddenly jumps, the speed loop can inject a false correction into both motors
- because the balance and speed terms are mixed together, a bad speed sample can contaminate the common stabilizing effort

The archived firmware shows a stronger defensive version of this idea:

- adaptive thresholds were computed from recent history
- abrupt jumps were compared against those thresholds
- samples that looked implausible were replaced by a median-like fallback instead of being trusted immediately

The cleaned controller is simpler, but the design lesson stayed the same: speed feedback is useful only if it is bounded, interpretable, and resistant to one-sample corruption.

## 8. ROS Command-Path Problem And The `/before_vel` Solution

### Problem

High-level navigation software expects to command a mobile base through something like `/cmd_vel`. A balancing robot is different. It cannot safely treat high-level velocity as a direct wheel command because the low-level controller still needs authority over body stabilization.

### Architecture Change

The simulation stack therefore uses:

```text
teleop or move_base
  -> /before_vel
  -> balancing controller
  -> /cmd_vel
  -> robot
```

Why this is important:

- `/before_vel` represents motion intent, not final actuator authority.
- The balancing controller remains the gatekeeper that translates intent into stable wheel action.
- This mirrors the physical robot philosophy, where the Arduino remains the real-time control authority close to the hardware.

This is not just a naming choice. It is the software-architecture version of the same lesson learned on the physical robot: do not let higher-level commands bypass the unstable system's low-level stabilizer.

## 9. What This Says About The Problem-Solving Approach

The strongest engineering story in the repository is not that one equation eventually balanced the robot. It is that the project repeatedly turned vague failure into smaller technical questions:

- Is the receiver noisy, or is the motor path wrong?
- Is the hall path wrong, or is ODrive firmware compatibility the issue?
- Is the robot falling because the gains are wrong, or because upright reference and current limits are wrong?
- Should navigation write directly to wheel motion, or should it write to a pre-balance intent channel?

That pattern led to concrete design responses:

- tester sketches for subsystem isolation
- filtered and state-gated RC input
- layered balance, speed, and steering control
- explicit safety supervision
- a separate ROS intent topic instead of direct final velocity control

## Short Version

If I had to summarize this project briefly for a reviewer or interview:

> The main challenge was not deriving one balancing equation in isolation, but making the full sensing, RC, motor-control, and safety loop trustworthy on real hardware. Receiver PWM noise was reduced through placement, routing, filtering, deadband, and engage persistence. ODrive runaway behavior was isolated toward the firmware or hoverboard hall path and was empirically stabilized by downgrading firmware. The final controller used LQR-style body-angle feedback, PID-like speed bias, steering and yaw damping, current limiting, tilt cutoff, and activation gating so the robot could balance and drive without letting any one noisy subsystem take over the actuators directly.
