# Physical Balance Control Algorithm

This document explains the structure of the real robot controller implemented in
[`physical_balance_controller.ino`](physical_balance_controller.ino).

The important idea is that the Arduino is not just passing commands through. It is the real-time safety and balance controller. RC input asks for motion, the IMU tells the robot how far it is leaning, ODrive feedback estimates wheel speed, and the final output is a current command to each wheel motor.

<p align="center">
  <img src="../../media/diagrams/physical_balance_control_loop.png" alt="Physical balance control loop" width="900">
</p>

## Control Responsibility

| Part | Role in the controller |
| --- | --- |
| FrSky X8R receiver | Provides `throttle`, `steering`, and `engage` PWM signals from the handheld controller |
| BNO055 IMU | Provides body tilt angle, tilt rate, and yaw rate |
| Arduino Mega 2560 | Runs filtering, safety checks, balance control, speed correction, steering correction, and motor-current mixing |
| ODrive 3.6 | Receives current commands and drives the two hub motors |
| Hub motor hall/encoder feedback | Provides wheel-speed information used for speed correction |

## Main Runtime Loop

The Arduino loop is split into small scheduled jobs using `Metro`.

| Job | Code path | Purpose |
| --- | --- | --- |
| Controller update | `LqrController()` then `ApplyMotorCommands()` | Reads sensors, calculates correction terms, sends motor current |
| Activation update | `EngageMotors()` | Uses the filtered engage switch and tilt safety flag to enter or leave ODrive closed-loop mode |
| Optional serial printing | `PrintRcSignals()` | Debug output for raw and filtered RC signal inspection |
| Serial command parsing | `cmd.parse_command()` | Allows gain tuning while the controller is running |

## Input Filtering

The RC receiver values are noisy enough that the controller filters them before use.

```text
filtered_pwm = alpha * raw_pwm + (1 - alpha) * previous_filtered_pwm
```

The sketch uses different filter strengths for different signals:

| Signal | Filter value | Why it matters |
| --- | --- | --- |
| `throttle` | `kAlpha_1 = 0.4` | Smooths forward/backward command without making driving feel too delayed |
| `steering` | `kAlpha_1 = 0.4` | Smooths left/right command |
| `engage` | `kAlpha_2 = 0.02` | Makes motor activation less sensitive to short PWM spikes |

After filtering, throttle and steering use a neutral deadband around the receiver center value. This prevents small stick noise from becoming unwanted motion.

## Safety Layer

The controller has two main safety gates before motors are allowed to stay active.

```text
motors_active = engage_request && !tilt_limit_exceeded
```

| Safety behavior | Implementation detail |
| --- | --- |
| Engage persistence | The engage PWM must remain above the threshold long enough to pass the persistence counter |
| Tilt cutoff | If body tilt exceeds `kTiltDisengageThresholdDegrees = 30`, the controller marks `tilt_limit_exceeded` and exits the balance calculation |
| Integral reset | Balance integral error is reset when motors are inactive or tilt safety is exceeded |
| Current limit | Final current commands are constrained to `-8A` to `+8A` before being sent to ODrive |

## Balance Term

The core balancing term uses the body angle from the BNO055 and the angular velocity from the gyroscope.

```text
theta     = body tilt angle + imu_angle_offset
theta_dot = body tilt angular velocity

balance_controller =
  (K_theta / 100)     * theta
- (K_theta_dot / 100) * theta_dot
+ (Ki / 100)          * integral_error
```

In the code this is named `balance_controller`. It is the term that tries to keep the robot body upright.

## What Kind Of Controller This Actually Is

The sketch comments call the controller `LQR`, and the balance term does use state-feedback-style gains on body angle and body angular velocity. In the deployed robot, the real controller is best understood as a layered controller rather than one isolated textbook regulator:

- an `LQR-style balance term` on `theta` and `theta_dot`
- an `integral correction` to reduce steady-state lean bias
- a `PID-like speed loop` that turns throttle request into a forward or backward balance bias
- a `steering and yaw-damping loop` that creates left-right current difference
- a `safety supervisor` that can refuse activation or drop the motors to idle

That distinction matters. A balancing robot is an unstable plant. The job is not only to compute one torque value, but also to decide when the robot is safe enough to apply torque at all, how aggressively it should accelerate, and how to keep steering commands from corrupting the upright loop.

## Speed Correction

The controller converts ODrive wheel-speed feedback into estimated linear velocity:

```text
v_left_or_right = motor_direction * wheel_speed * 2 * PI * wheel_radius
```

Throttle input becomes a target speed:

```text
target_speed = throttle_input * maxspeed
```

Then a PID-like speed correction is calculated:

```text
speed_error = target_speed - average_wheel_speed

speed_control =
  kpspeed * speed_error
+ kispeed * integral_speed_error
- kdspeed * derivative_speed_error
```

This term lets RC throttle ask the robot to move forward or backward while the balance term still keeps it upright.

## Steering Correction

Steering combines the filtered receiver steering signal with the IMU yaw rate:

```text
steering_controller = kpSteer * steering + kdSteer * yaw_rate
```

The result is mixed with opposite signs on the left and right motor commands, so the robot can turn while balancing.

## Motor Mixing

The final current request is built from three terms:

```text
motor_0 = balance_controller - speed_control - steering_controller
motor_1 = balance_controller - speed_control + steering_controller
```

Then a small optional `tanh()` compensation term can be added to reduce low-speed bias or friction effects:

```text
motor_0 += adjustment_value * tanh(scaling_factor * wheel_speed_0)
motor_1 -= adjustment_value * tanh(scaling_factor * wheel_speed_1)
```

Finally, both commands are clamped and sent to ODrive:

```text
current_command = constrain(motor_command, -kMaxAbsCurrent, kMaxAbsCurrent)
ODrive.SetCurrent(axis, motor_direction * current_command)
```

## Why This Structure Was Necessary

A normal differential-drive robot can often treat `throttle -> wheel velocity` as the main problem. This robot could not. Forward motion had to be created by intentionally shifting the balance point without letting the body tip too far, and steering had to be introduced without destroying the stabilization term already fighting gravity.

That is why the controller is layered in this order:

1. Estimate whether the body is still inside a safe tilt window.
2. Compute the stabilizing balance torque from `theta` and `theta_dot`.
3. Add a speed correction that biases the robot to roll forward or backward.
4. Add a steering correction with opposite sign on the two wheels.
5. Clamp the final current and send it to ODrive only if the safety state allows it.

This structure is what turns the robot from a bench motor experiment into a driveable self-balancing system.

## How The Algorithm Solved Real Problems

The control structure is tied directly to the troubleshooting history:

- `RC noise and PWM spikes`: raw receiver PWM is filtered before it affects throttle, steering, or engage state.
- `Accidental motor activation`: the engage switch is not trusted instantly; it must pass a persistence filter first.
- `Fall risk during tuning`: the tilt cutoff can force the control path into a safe inactive state.
- `Aggressive current bursts`: current commands are clamped before being sent to ODrive.
- `Forward motion while balancing`: throttle does not bypass the balance term; it becomes a target speed that modifies the equilibrium indirectly.
- `Turning instability`: steering is mixed as a differential term so the robot can rotate without throwing away the common balance effort.

In other words, the algorithm is not only a control law. It is also the distilled record of what had to be added so the real hardware would stop behaving like a collection of fragile subsystems.

## Tuning Symptoms And Tradeoffs

| If this part is wrong | Likely behavior | Why it happens |
| --- | --- | --- |
| `K_theta` too low | slow falling or weak recovery | body angle error is not corrected strongly enough |
| `K_theta` too high | sharp oscillation or chatter | angle error is over-amplified into current command |
| `K_theta_dot` too low | overshoot after a lean | body angular velocity is not damped enough |
| `K_theta_dot` too high | stiff, noisy, or hesitant response | gyro-rate term suppresses motion too aggressively |
| `Ki` too high | creeping or slow oscillation | accumulated bias correction starts to dominate |
| `kpspeed`, `kispeed`, `kdspeed` poorly tuned | surging, hunting, or poor speed tracking | the robot is moving the balance point incorrectly |
| `kpSteer` or `kdSteer` too high | unstable turning or wheel asymmetry | steering correction overwhelms the common balance term |
| RC deadband too small | wheels react to stick noise near neutral | small PWM drift is treated like user intent |
| current clamp too high during early tuning | hard falls or violent recovery | the actuator can inject too much energy before the operator reacts |

## Tuning Interface

The sketch exposes a small serial tuning interface through `Command_processor`.

| Command | Parameter changed | Meaning |
| --- | --- | --- |
| `q` | `K_theta` | Body-angle balance gain |
| `w` | `Ki` | Balance integral gain |
| `e` | `K_theta_dot` | Body angular-velocity gain |
| `r` | `imu_angle_offset` | Mechanical/sensor angle offset |
| `t` | `adjustment_value` | Extra low-speed compensation amount |
| `y` | `scaling_factor` | Shape of the `tanh()` compensation |
| `a` | `kpspeed` | Speed-loop proportional gain |
| `s` | `kispeed` | Speed-loop integral gain |
| `d` | `kdspeed` | Speed-loop derivative gain |

## Why This Structure Matters

A balancing robot cannot treat throttle like a normal wheeled robot. The controller must first keep the body upright, then bias that balance point to create forward/backward motion, and finally add steering as a difference between the two wheels.

That is why the firmware is structured as:

```text
RC request + IMU state + wheel feedback
  -> safety checks
  -> balance correction
  -> speed correction
  -> steering correction
  -> left/right current commands
  -> ODrive
```

This is also why the real robot keeps the low-level control loop on Arduino instead of depending on ROS for balance timing.
