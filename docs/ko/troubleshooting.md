# 프로젝트 트러블슈팅 요약

[English](../troubleshooting.md) | 한국어

이 문서는 프로젝트 뒤에 있는 engineering trouble log입니다. 가장 어려웠던 문제를 어떻게 더 작은 check로 나누었고, 그 결과 어떤 control 또는 architecture change를 적용했는지 설명합니다. Code나 notes로 명확히 뒷받침되는 부분은 구체적으로 쓰고, 정확한 root cause를 완전히 증명하지 못한 부분은 조심스럽게 표현했습니다.

## 이 문서를 읽는 방법

- `Confirmed`: firmware, tester sketches, public docs, 반복 가능한 project notes로 직접 뒷받침되는 내용.
- `Likely inference`: 실험실 수준으로 증명하지는 못했지만, project material을 가장 잘 설명하는 해석.
- `Not claimed`: 탐색했을 수는 있지만 finished final result로 제시하지 않는 내용.

## 이 요약의 근거

이 요약은 주로 다음 자료를 바탕으로 합니다.

- `firmware/physical_balance_controller/physical_balance_controller.ino`
- `firmware/physical_balance_controller/control_algorithm.md`
- `firmware/testers/*`
- `archive/arduino_firmware/*`
- `ros_ws/src/*`
- `docs/*.md`

ODrive hoverboard bring-up, RF/EMI behavior 같은 부분에서는 외부 reference도 도움이 되었지만, 이 문서의 주된 목적은 이 프로젝트가 실제로 무엇을 했는지 설명하는 것입니다.

## High-Level Problem Map

| Area | Main symptom | How it was isolated | Main mitigation or design response | How sure I am |
| --- | --- | --- | --- | --- |
| RC receiver path | Wheel twitch와 PWM spike | Serial monitoring, wiring changes, metal proximity sensitivity, foil experiment | Receiver placement 개선, twisted signal-ground routing, filtering, deadband, engage persistence | Mostly confirmed, with some inference |
| ODrive path | 간헐적인 uncontrolled motor behavior | Hall test, receiver test, Arduino path checks, ODrive-focused tester sketches, firmware downgrade result | Firmware downgrade와 controller safety gating 강화 | Mostly confirmed, with some inference |
| Physical balancing | Fall risk와 extreme tuning sensitivity | Tethered practice, staged subsystem bring-up, gain iteration | LQR-style balance term, speed loop, steering loop, current clamp, tilt cutoff | Confirmed |
| IMU repeatability | Balance point drift와 inconsistent upright reference | Calibration checks, saved offsets, angle offset tuning | BNO055 calibration handling과 explicit `imu_angle_offset` | Confirmed |
| Navigation command path | High-level ROS command가 balancing robot을 직접 구동하기 어려움 | Simulation controller design과 `/before_vel` pipeline | Navigation intent와 final low-level balance output 분리 | Confirmed |

## 실제 문제를 해결하기 위해 도입한 Control Techniques

최종 controller는 하나의 control equation만이 아닙니다. 실제 hardware failure와 tuning lesson에서 나온 countermeasure stack입니다.

| Technique | Problem it addresses | Where it shows up in code |
| --- | --- | --- |
| Low-pass filtering on RC PWM | 짧은 spike와 noisy command edge | `filtered_throttle_pwm`, `filtered_steering_pwm`, `filtered_engage_pwm`, `kAlpha_1`, `kAlpha_2` |
| Neutral offset and deadband | Receiver center drift로 인한 unintended motion | `kSteeringPwmOffset`, `kThrottlePwmOffset`, `kControlThreshold` |
| Engage persistence | Noisy PWM으로 인한 false motor enable/disable | `engage_signal_persistence`, `EngageMotors()` |
| Tilt cutoff | Robot이 hard fall posture에 도달하기 전에 motor command 차단 | `kTiltDisengageThresholdDegrees`, `tilt_limit_exceeded` |
| Current clamp | Violent recovery 또는 runaway command 제한 | `kMaxAbsCurrent`, `ApplyMotorCommands()` |
| LQR-style balance term | 불안정한 body를 세우는 main stabilizing feedback | `theta`, `theta_dot`, `integral_error`로 만든 `balance_controller` |
| PID-like speed correction | Forward/backward motion이 balancing과 공존하도록 함 | `speed_control`, `kpspeed`, `kispeed`, `kdspeed` |
| Steering plus yaw damping | Common balance effort를 깨지 않으면서 turning | `steering_controller`, `kpSteer`, `kdSteer` |
| Separate ROS intent topic | High-level navigation이 balance logic을 우회하지 않도록 함 | `ros_ws/src/balance_robot_control`, `ros_ws/src/navigation`의 `/before_vel` design |

## 1A. 왜 Filtering이 필요했는가

Balancing robot project에서 하기 쉬운 실수 중 하나는 filtering을 cosmetic cleanup 정도로 보는 것입니다. 이 프로젝트에서 filter는 여러 signal이 raw로 쓰기에는 충분히 noisy했고, 그 noise가 control problem으로 이어질 수 있었기 때문에 존재했습니다.

### Underlying Problem

Controller는 서로 다른 방식으로 취약한 signal에 의존했습니다.

- RC PWM은 짧게 jump하거나 neutral point 주변에서 drift할 수 있었습니다.
- ODrive wheel-speed feedback은 sudden spike, parse irregularity, implausible jump를 포함할 수 있었습니다.
- IMU-derived body motion은 유용했지만 calibration offset, mounting bias, mechanical vibration과 함께 다뤄야 했습니다.

이 로봇은 stable plant를 제어하는 것이 아니었습니다. 짧은 bad sample 하나도 단순 logging nuisance가 아니라 equilibrium point, torque request, motor activation state를 직접 바꿀 수 있었습니다.

### Filter가 없으면 생길 수 있는 문제

Raw signal을 그대로 쓰면 다음 failure mode가 생길 수 있었습니다.

- 짧은 RC PWM spike가 실제 throttle 또는 steering command처럼 보일 수 있음
- Neutral 주변의 작은 center drift가 robot creep 또는 계속된 motion request로 바뀔 수 있음
- Noisy engage channel이 잘못된 시점에 motor를 arm/disarm할 수 있음
- Spiky wheel-speed estimate가 speed loop를 왜곡해 과장된 corrective current를 만들 수 있음
- Bad encoder/ODrive read 하나가 measurement fault가 아니라 실제 state change로 해석될 수 있음

그래서 하나의 generic filter에 의존하지 않았습니다. Failure mode별로 다른 countermeasure를 넣었습니다.

### 도입한 것

RC input에는 다음을 적용했습니다.

- `throttle`, `steering`, `engage`에 exponential low-pass filtering
- 정확한 `1500 us` center를 가정하지 않는 neutral offset
- Neutral region 주변 deadband
- Engage signal이 motor state를 바꾸기 전 persistence logic

Wheel 또는 ODrive feedback에는 다음을 적용했습니다.

- Older firmware에서 repeated defensive parsing
- Archived firmware에서 median-style outlier replacement와 adaptive threshold checks
- Cleaned controller에서 first-order smoothing of wheel-speed estimates
- Noisy speed feedback이 무한히 누적되지 않도록 constrained integrators

### 왜 이것이 Control Design Decision인가

중요한 점은 filtering이 post-processing detail이 아니라 control architecture의 일부였다는 것입니다. 이 프로젝트에서 배운 핵심은 `measurement trust`를 명시적으로 설계해야 한다는 점입니다. 어떤 signal을 balance, speed bias, steering, motor activation에 영향을 줄 만큼 믿을 수 있는지 결정한 뒤에야 controller가 의미를 갖습니다.

## 1. RC Receiver PWM Spike And Wheel Twitch

### Symptom

Physical driving과 balancing bring-up 중 의도하지 않은 순간에 wheel twitch가 생길 수 있었습니다. Arduino serial output을 통해 FrSky X8R PWM reading이 갑자기 jump할 수 있다는 것도 확인했습니다.

### What Was Checked

- `receiver_pwm_test.ino`로 receiver PWM을 직접 확인했습니다.
- Wiring과 receiver-side connection을 여러 번 수정했습니다.
- Receiver placement가 metal structure 근처에 있을 때 문제가 눈에 띄게 달라졌습니다.
- Nearby conductive material이 signal path에 영향을 주는지 확인하기 위해 aluminum-foil troubleshooting experiment를 시도했습니다.

### What It Most Likely Meant

가장 강한 설명은 RF placement sensitivity와 single-ended PWM line의 일반적인 signal-integrity problem이 함께 작용했다는 것입니다.

- Receiver와 antenna behavior가 conductive structure 근처에서 나빠졌습니다.
- Foil experiment는 spike를 줄이지 않고 더 악화시켰는데, 이는 RF environment를 fixing하기보다 detuning/disturbing한 상황과 잘 맞습니다.
- 길거나 느슨하게 routing된 PWM line은 induced noise와 ground-reference problem에 취약합니다.

따라서 이것은 단순 software bug가 아니라 `receiver placement plus wiring integrity` 문제로 보는 것이 가장 적절합니다.

### What Was Changed

Hardware-side mitigation:

- Receiver와 antenna를 metal structure, power wiring, high-current motor-control region에서 떨어뜨립니다.
- 각 PWM signal을 ground return과 최대한 가까이 routing합니다.
- Receiver placement를 control system의 일부로 취급합니다.

Firmware-side mitigation:

- Throttle, steering, engage PWM을 사용하기 전에 low-pass filtering합니다.
- 모든 channel center가 정확히 `1500 us`라고 가정하지 않고 neutral offset을 사용합니다.
- 작은 PWM drift가 drive command가 되지 않도록 deadband를 적용합니다.
- Engage signal이 motor를 activate하기 전 persistence를 요구합니다.

### Why The Control Changes Matter

이 변화가 없으면 balance controller는 noisy RC pulse를 실제 intent로 해석합니다. 일반 wheeled robot에서는 짧은 jerk 정도일 수 있지만, self-balancing robot에서는 더 위험합니다. False throttle 또는 steering command는 이미 unstable한 plant의 target equilibrium을 바꿉니다. 그래서 RC filtering은 comfort feature가 아니라, closed loop가 일어설 만큼 믿을 수 있게 만드는 요소였습니다.

## 2. ODrive Runaway And Sudden Uncontrolled Motor Behavior

### Symptom

프로젝트에서 가장 심각한 문제 중 하나는 ODrive path를 통한 intermittent uncontrolled motor behavior였습니다. 로봇이 갑자기 가속하거나 깔끔하게 멈추지 못하는 현상은 balancing test에서 매우 위험한 failure mode입니다.

### Isolation Strategy

프로젝트는 처음부터 전체 로봇을 탓하지 않고 문제를 더 작은 path로 나누었습니다.

- `hall_sensor_test.ino`로 hall-state transition과 illegal state를 확인했습니다.
- `receiver_pwm_test.ino`로 RC PWM 자체가 정상인지 확인했습니다.
- `odrive_receiver_test.ino`로 receiver command와 ODrive behavior 사이의 integration path를 확인했습니다.
- `motor_current_test.ino`로 문제를 direct current-command behavior까지 줄였습니다.
- 최종 physical controller는 ODrive가 항상 active state에 있어도 된다고 가정하지 않고 explicit engage/safety state management를 추가했습니다.

이 isolation process는 문제가 random gain tuning이 아니라 system-debugging problem으로 다뤄졌음을 보여줍니다.

### Most Honest Conclusion

가장 방어 가능한 공개 표현은 다음과 같습니다.

> The runaway issue was isolated toward the ODrive firmware or hoverboard hall-sensor bring-up path rather than being proven as a receiver-side or Arduino-side PWM problem.

이 표현이 적절하다고 보는 이유는 다음과 같습니다.

- Hall testing은 다른 쪽을 계속 조사해도 될 만큼 정상으로 보였습니다.
- Receiver PWM issue는 존재했지만, ODrive-specific symptom pattern을 완전히 설명하지는 못했습니다.
- Firmware `0.5.1` 이후 hoverboard hall behavior 관련 ODrive community pattern이 있었습니다.
- Firmware downgrade 이후 이 프로젝트에서 runaway symptom이 사라졌습니다.

정확한 firmware-level root cause를 formal하게 증명하지는 못했기 때문에, `fully root-caused firmware bug`보다 `empirically solved by firmware downgrade`라고 쓰는 것이 더 정직합니다.

### What Was Introduced In Response

Downgrade 이후에도 ODrive를 always-trusting black box로 다루지 않도록 controller architecture를 강화했습니다.

- `EngageMotors()`가 axis를 `CLOSED_LOOP_CONTROL`과 `IDLE` 사이에서 명시적으로 전환합니다.
- 큰 tilt는 `tilt_limit_exceeded`를 force하고 motor activation을 막을 수 있습니다.
- Final current는 `ODrive.SetCurrent` 호출 전에 `+/-8 A`로 clamp됩니다.
- Motors inactive 또는 tilt safety exceeded 상태에서는 integral term을 reset합니다.

이것들은 단순 cosmetic safety check가 아닙니다. Hardware path가 pure balance equation만으로는 감당하기 어려운 방식으로 misbehave할 수 있음을 보여준 뒤 추가한 supervisory control feature입니다.

## 3. 왜 단순 Controller로는 부족했는가

Physical robot은 throttle을 direct wheel speed로 처리하는 방식만으로 안정화할 수 없었습니다. Balancing robot은 unstable mechanical system입니다. Controller는 먼저 body를 upright하게 유지해야 하고, 그 upright condition을 의도적으로 bias해서 motion을 만들고, 다시 wheel effort를 split해서 steering해야 합니다.

최종 firmware는 다음과 같은 layered controller로 동작합니다.

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

이 구조는 [control_algorithm.md](../../firmware/physical_balance_controller/control_algorithm.md)에 시각적으로 정리되어 있습니다.

## 4. 실제 로봇 문제를 해결하기 위해 채택한 Control Algorithm

### 4.1 Balance Term

최종 controller의 core stabilizing term은 다음과 같습니다.

```text
balance_controller =
  (K_theta / 100)     * theta
- (K_theta_dot / 100) * theta_dot
+ (Ki / 100)          * integral_error
```

해석:

- `theta`는 BNO055에서 얻은 body tilt angle이며, `imu_angle_offset`으로 보정됩니다.
- `theta_dot`은 gyroscope에서 얻은 body angular velocity입니다.
- `integral_error`는 active safe state에서만 tilt error를 누적합니다.

이것이 중요한 이유:

- `theta`는 gravity problem입니다. Body가 기울면 wheel이 center of mass 아래로 이동해야 합니다.
- `theta_dot`은 damping problem입니다. 이것이 없으면 robot은 맞는 방향으로 반응하더라도 크게 overshoot할 수 있습니다.
- `integral_error`는 bias problem입니다. Small mounting offset, unequal friction, slight sensor bias가 있으면 robot이 practical upright point 주변에 settle하지 못할 수 있습니다.

그래서 이 controller는 단순 `PID`라기보다 `LQR-style`이라고 설명하는 것이 적절합니다. Angle과 angular velocity에 대한 explicit state feedback을 사용하고, 실제 hardware가 요구한 추가 term을 더했습니다.

### 4.2 Speed Loop

Forward/backward motion은 raw wheel command로 적용하지 않습니다. Target speed로 바꾼 뒤 correction term으로 변환합니다.

```text
target_speed = throttle_input * maxspeed
speed_error = target_speed - average_wheel_speed

speed_control =
  kpspeed * speed_error
+ kispeed * integral_speed_error
- kdspeed * derivative_speed_error
```

필요했던 이유:

- Balancing robot은 four-wheel robot처럼 단순히 "drive forward"할 수 없습니다.
- 앞으로 움직이려면 안정화 영역 안에 머무르면서 controlled forward bias를 허용해야 합니다.
- Speed loop는 balance controller를 대체하지 않고 operating point를 이동시킵니다.

이것은 저장소에서 가장 강한 engineering decision 중 하나입니다. Project가 `motion request`와 `actuator authority`를 혼동하지 않았음을 보여줍니다.

### 4.3 Steering Loop

Steering은 별도로 계산합니다.

```text
steering_controller = kpSteer * steering + kdSteer * yaw_rate
```

그 다음 steering term은 반대 부호로 mix됩니다.

```text
motor_0 = balance_controller - speed_control - steering_controller
motor_1 = balance_controller - speed_control + steering_controller
```

이 구조가 중요한 이유:

- Balance term은 두 wheel 모두에 같은 방향으로 필요한 common-mode effort입니다.
- Steering term은 두 wheel 사이에 반대로 들어가야 하는 differential effort입니다.
- 이렇게 mix하면 shared stabilizing action을 유지하면서 turn할 수 있습니다.

RC receiver에서 left/right wheel command를 독립적으로 직접 넣는 것보다 이 구조가 balancing robot에 훨씬 잘 맞습니다.

### 4.4 Safety Supervisor

Controller는 스스로를 언제 믿지 않아야 하는지도 supervision하기 때문에 credible합니다.

Key logic:

- `filtered_engage_pwm`은 activation 전에 persistence check를 통과해야 합니다.
- `abs(theta)`가 `kTiltDisengageThresholdDegrees`를 넘으면 `tilt_limit_exceeded`가 설정됩니다.
- Motors는 control state에 계속 남아 있지 않고 `IDLE`로 밀어낼 수 있습니다.
- Final current는 `kMaxAbsCurrent`로 constrain됩니다.

Physical tilt cutoff는 protection bracket이 약 `28 deg`에서 바닥에 닿기 때문에 `30 deg`로 설정했습니다. 즉 임의의 control number가 아니라 practical damage-prevention threshold였습니다.

Balancing robot에서 이런 supervisory rule은 control design의 일부입니다. "진짜 알고리즘"을 감싸는 protective wrapper가 아니라, actuation이 허용되는 조건까지 포함해야 real algorithm이 됩니다.

## 5. Balance Tuning Strategy And Fall-Risk Management

### Practical Problem

좋은 equation이라도 gain을 full robot에서 너무 일찍 tuning하면 크게 실패할 수 있습니다. Balancing robot은 body angle, wheel speed, current response, mass distribution, sensor bias가 강하게 coupling되어 있습니다. 그래서 early tuning을 통제하지 않으면 engineering이라기보다 crash testing에 가까워집니다.

### What The Project Did Instead

- Full-body balancing 전에 subsystem을 별도로 bring-up했습니다.
- Motor와 hall feedback은 bench test로 먼저 확인했습니다.
- Untethered physical driving 전에 tethered practice를 사용했습니다.
- Early tuning 중 current를 제한했습니다.
- Robot이 valid active state가 아닐 때 integral accumulation을 reset하거나 suppress했습니다.

### Why This Is A Real Control Strategy

Tuning process 자체가 solution의 일부였습니다. 이 프로젝트는 하나의 magic gain set을 찾아서 해결한 것이 아니라 search space를 제한해서 해결했습니다.

- Raw RC command가 motor current로 직접 들어가지 않음
- Noisy center value를 완전히 믿지 않음
- Experiment 중 unlimited current를 허용하지 않음
- Fall angle 이후에는 integration을 계속하지 않음
- Subsystem 하나가 정상이라고 full closed loop가 정상이라고 가정하지 않음

이 차이가 mathematically plausible controller와 실제 bench-to-floor transition을 견디는 controller의 차이입니다.

## 6. IMU Calibration And Upright Reference Management

### Problem

IMU가 robot의 실제 upright posture와 맞지 않으면 controller는 잘못된 angle을 기준으로 안정화하려고 합니다. Balancing robot에서는 creeping, persistent correction, asymmetric recovery, immediate fall behavior로 나타날 수 있습니다.

### Evidence In The Firmware

- BNO055가 감지되지 않으면 code가 halt합니다.
- EEPROM을 통한 calibration save/load helper가 있습니다.
- Calibration status를 print하고 확인할 수 있습니다.
- Controller는 explicit `imu_angle_offset`을 사용합니다.

### Interpretation

프로젝트는 `upright`가 순수 theoretical reference가 아니라는 것을 배웠습니다. Upright는 sensor alignment, mounting angle, calibration state에 의존하는 calibrated operating condition입니다. 그래서 controller는 raw IMU value를 직접 믿지 않고 calibration handling과 explicit angle offset을 모두 포함합니다.

## 7. Wheel-Speed Feedback Trust And Serial Robustness

ODrive feedback path에서도 같은 defensive mindset이 보입니다.

- Legacy firmware는 ODrive feedback 주변에서 repeated reads, smoothing, outlier handling을 사용했습니다.
- Final controller는 wheel-speed read 전후로 `Serial3`를 flush합니다.
- Speed-related integral은 noisy feedback이 unbounded correction으로 누적되지 않도록 constrain됩니다.

이것은 balancing system에서 feedback quality가 controller stability의 일부라는 점을 인식했다는 의미입니다. Mathematically correct control law라도 velocity estimate가 delayed, spiky, inconsistently parsed이면 실패할 수 있습니다.

### What Filtering Problem This Solved

Wheel-speed path의 filtering은 plot을 보기 좋게 만드는 목적이 아니었습니다. Control problem을 해결하기 위한 것이었습니다.

- Speed loop는 average wheel velocity로 forward/backward bias를 얼마나 줄지 결정합니다.
- Velocity estimate가 갑자기 jump하면 speed loop가 false correction을 두 motor에 주입할 수 있습니다.
- Balance와 speed term이 섞여 있기 때문에 bad speed sample은 common stabilizing effort까지 오염시킬 수 있습니다.

Archived firmware는 이 아이디어의 더 강한 defensive version을 보여줍니다.

- Recent history로 adaptive threshold를 계산했습니다.
- Abrupt jump를 threshold와 비교했습니다.
- Implausible sample은 즉시 믿지 않고 median-like fallback으로 대체했습니다.

Cleaned controller는 더 단순하지만, design lesson은 그대로입니다. Speed feedback은 bounded, interpretable, one-sample corruption에 resistant할 때만 유용합니다.

## 8. ROS Command-Path Problem And The `/before_vel` Solution

### Problem

High-level navigation software는 보통 `/cmd_vel` 같은 topic을 통해 mobile base에 명령을 보냅니다. 하지만 balancing robot은 다릅니다. Low-level controller가 body stabilization 권한을 계속 가져야 하기 때문에 high-level velocity를 direct wheel command로 다루면 안전하지 않습니다.

### Architecture Change

Simulation stack은 다음 구조를 사용합니다.

```text
teleop or move_base
  -> /before_vel
  -> balancing controller
  -> /cmd_vel
  -> robot
```

중요한 이유:

- `/before_vel`은 motion intent이지 final actuator authority가 아닙니다.
- Balancing controller는 intent를 stable wheel action으로 변환하는 gatekeeper로 남습니다.
- 이는 physical robot philosophy와 같습니다. Arduino가 hardware 가까이에서 real-time control authority를 유지합니다.

이것은 단순 naming choice가 아닙니다. Physical robot에서 배운 같은 lesson의 software-architecture version입니다. High-level command가 unstable system의 low-level stabilizer를 우회하게 두면 안 됩니다.

## 9. What This Says About The Problem-Solving Approach

이 저장소에서 가장 강한 engineering story는 어떤 equation 하나가 결국 robot을 세웠다는 이야기가 아닙니다. Vague failure를 더 작은 technical question으로 계속 바꾼 과정입니다.

- Receiver가 noisy한가, 아니면 motor path가 잘못되었는가?
- Hall path가 문제인가, 아니면 ODrive firmware compatibility 문제인가?
- Robot이 넘어지는 이유가 gain 때문인가, upright reference/current limit 때문인가?
- Navigation이 wheel motion에 직접 써야 하는가, 아니면 pre-balance intent channel에 써야 하는가?

이 패턴은 구체적인 design response로 이어졌습니다.

- Subsystem isolation을 위한 tester sketches
- Filtered and state-gated RC input
- Layered balance, speed, steering control
- Explicit safety supervision
- Direct final velocity control 대신 separate ROS intent topic

## Short Version

Reviewer 또는 interview에서 짧게 요약하면 다음과 같습니다.

> The main challenge was not deriving one balancing equation in isolation, but making the full sensing, RC, motor-control, and safety loop trustworthy on real hardware. Receiver PWM noise was reduced through placement, routing, filtering, deadband, and engage persistence. ODrive runaway behavior was isolated toward the firmware or hoverboard hall path and was empirically stabilized by downgrading firmware. The final controller used LQR-style body-angle feedback, PID-like speed bias, steering and yaw damping, current limiting, tilt cutoff, and activation gating so the robot could balance and drive without letting any one noisy subsystem take over the actuators directly.
