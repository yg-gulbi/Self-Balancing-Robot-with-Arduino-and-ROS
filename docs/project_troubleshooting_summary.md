# Project Troubleshooting Summary

이 문서는 프로젝트 검수용 문제 총정리이다. 공개 README보다 더 솔직하게, 프로젝트를 진행하면서 실제로 부딪혔거나 코드와 문서에서 강하게 추정되는 문제를 모았다.

분류 기준:

- `확인`: 문서, 코드, 테스트 스케치, 사용자의 추가 설명에서 직접 확인되는 내용
- `추정`: 코드 구조나 수정 흔적상 가능성이 큰 내용
- `미확정`: 원인은 좁혀졌지만 최종 root cause를 완전히 증명하지 못한 내용

## Evidence Scope

확인한 범위:

- 공개 문서: `README.md`, `docs/*.md`
- 실제 제어 펌웨어: `firmware/physical_balance_controller/physical_balance_controller.ino`
- 서브시스템 테스트: `firmware/testers/*`
- 과거 펌웨어: `archive/legacy_firmware/*`
- ROS/Gazebo 코드: `ros_ws/src/*`
- 과거 ROS 실험: `archive/legacy_code/*`
- 회수 자료 설명: `archive/raw_documents/README.md`, `archive/media_manifest.md`

제한:

- `archive/raw_documents/로봇 시행착오.pdf`는 보존용 raw 자료로 존재하지만, 현재 로컬 환경에서 PDF 텍스트 추출/OCR 도구를 사용할 수 없어 본문을 직접 검증하지 못했다. 기존 문서상 이 PDF는 `docs/development_process.md`로 요약되었다고 되어 있으므로, 여기서는 공개 Markdown과 코드 증거를 우선으로 사용했다.

외부 근거:

- ODrive 0.5.6 Getting Started: https://docs.odriverobotics.com/v/0.5.6/getting-started.html
- ODrive 0.5.6 Hoverboard Guide: https://docs.odriverobotics.com/v/0.5.6/hoverboard.html
- ODrive Community, "Hoverboard motors not working after release 0.5.1": https://discourse.odriverobotics.com/t/hoverboard-motors-not-working-after-release-0-5-1/9393
- Analog Devices, twisted pair/loop area and EMI explanation: https://www.analog.com/en/resources/analog-dialogue/articles/about-wire.html
- Analog Devices, "Use a Twist and Other Popular Wires to Reduce EMI/RFI": https://www.analog.com/en/resources/technical-articles/2022/07/16/10/48/use-a-twist-and-other-popular-wires-to-reduce-emirfi.html
- Texas Instruments, AN-1811 Bluetooth Antenna Design: https://www.ti.com/lit/an/snoa519b/snoa519b.pdf
- Silicon Labs, UG252 antenna keep-out discussion: https://www.silabs.com/documents/public/user-guides/ug252-lighting-rd.pdf

## High-Level Problem Map

| Area | Problem | Evidence level | Short conclusion |
| --- | --- | --- | --- |
| RC receiver | Wheel twitch / PWM spike | 확인 | receiver PWM가 금속 구조물과 배선 노이즈에 민감했고, twisted signal-ground wiring으로 크게 완화된 것으로 정리 가능 |
| ODrive | Intermittent runaway / motor does not stop | 확인 + 미확정 | hall/receiver/Arduino output보다 ODrive firmware or hoverboard hall calibration path 쪽으로 격리됨; firmware downgrade로 해결됐으나 내부 원인은 미확정 |
| Hall sensor | Hall state validation needed | 확인 | hall sensor tester가 있고, illegal state 감시 구조가 있음; ODrive 문제와 분리 진단한 근거로 사용 가능 |
| ODrive serial feedback | Encoder/speed read spikes or parse fragility | 확인 + 추정 | legacy code에 adaptive threshold/median smoothing, serial flush, duplicate read가 있어 ODrive serial feedback 안정화 시도가 있었음 |
| Balance control | Tuning sensitivity and fall risk | 확인 | tethered practice, current limit, tilt cutoff, LQR/PID tuning 흔적이 있음 |
| IMU | Calibration repeatability | 확인 | BNO055 calibration save/load/check code와 angle offset이 있음 |
| RC engage safety | False engage/disengage | 확인 | PWM low-pass, deadband, engage persistence filter가 있음 |
| ROS command path | `move_base` command cannot directly drive balancing robot | 확인 | `/before_vel` design으로 navigation command와 final `/cmd_vel`를 분리함 |
| Gazebo simulation | Reset/time jump/PID instability | 확인 | reset detection, world reset scripts, fall penalty optimizer가 있음 |
| Physical autonomy | Real-world SLAM/navigation not fully completed | 확인 | docs에서 partial로 명시, real-world launch traces only |
| Repository | Reproducibility and evidence cleanup limits | 확인 | third-party deps not vendored, raw evidence external, absolute map paths, historical D435 names |

## 1. FrSky Receiver PWM Spike And Wheel Twitch

### Symptom

확인된 사용자 설명:

- 주행 중 바퀴가 한 번씩 튀는 문제가 있었다.
- 리시버 신호를 Arduino Serial로 확인할 때 값이 계속 튀었다.
- 선 수정, 리시버 내부 연결 부위 변경 등 여러 방법을 시도했지만 실패했다.
- 철판 위에 리시버를 두고 실험하자 신호가 갑자기 크게 튀었다.
- 금속판 근처에서 신호가 튀는 원인을 좁히기 위해 알루미늄 호일로 리시버를 감싸 보는 추가 실험을 했고, 신호가 더 심하게 튀었다.
- 철판에서 멀리 떨어뜨리면 좋아졌지만, 그래도 한 번씩 튀었다.
- 인터넷 검색 후 선을 꼬아서 배선했고, 이후 잘 동작했다.

### Local evidence

- `docs/research_and_design_decisions.md`는 FrSky receiver interference를 별도 조사 항목으로 두고, wire length, poor joins, grounding, shielding, twisted signal-ground routing, ferrite beads, filters를 noise factor로 정리한다.
- `docs/experiments.md`는 FrSky X8R receiver testing과 receiver interference troubleshooting을 별도 실험으로 기록한다.
- `firmware/testers/receiver_pwm_test/receiver_pwm_test.ino`는 receiver PWM pulse width를 Arduino Serial로 직접 출력하는 테스트이다. 사용자가 말한 "시리얼로 확인할 때 신호가 튄 문제"와 맞다.
- `firmware/physical_balance_controller/physical_balance_controller.ino`는 throttle/steering/engage PWM에 low-pass filter를 적용한다. `kAlpha_1 = 0.4`, `kAlpha_2 = 0.02`가 있고, `filtered_throttle_pwm`, `filtered_steering_pwm`, `filtered_engage_pwm`를 사용한다.
- 같은 파일은 neutral offset과 deadband를 둔다. `kSteeringPwmOffset = 1492`, `kThrottlePwmOffset = 1488`, `kControlThreshold = 50`은 수신기 중립값이 정확히 1500us에 고정되지 않고 흔들릴 수 있음을 전제로 한다.
- engage 신호는 즉시 ON/OFF하지 않고 `engage_signal_persistence`를 `-3..3` 범위로 누적한다. 짧은 PWM 튐 때문에 모터 상태가 바로 바뀌지 않도록 만든 구조이다.
- `archive/legacy_firmware/experimental_balance_controller_imu_ros.ino`에는 "Avoid false positive disengagements due to noisy PWM by persistence filtering."라는 주석이 직접 존재한다. 즉 noisy PWM 문제는 실제 디버깅 포인트였다.

### Technical explanation

가장 그럴듯한 원인은 두 가지가 겹친 것이다.

첫째, Arduino가 읽는 RC PWM은 pulse width를 `micros()`로 재는 single-ended digital signal이다. 신호선과 GND가 멀리 벌어져 있거나 길게 늘어지면 그 둘이 만든 루프 면적이 커지고, 근처 모터/ODrive/전원선/금속 구조물에서 생기는 자기장 변화가 유도 전압을 만들 수 있다. Analog Devices는 유도 전압이 loop area에 비례하므로 loop area를 줄여야 하고, 배선에서는 twisted pair가 그 방법이라고 설명한다. 또한 twisted pair는 외부 간섭이 두 선에 비슷하게 들어오게 만들어 RFI/EMI cancel 효과를 낸다고 설명한다.

둘째, FrSky X8R/Taranis Q X7 계열은 2.4GHz RF link를 사용한다. 2.4GHz 안테나는 주변 금속, ground plane, shielding, 손, 케이스 같은 물체에 의해 impedance matching과 radiation pattern이 바뀔 수 있다. TI의 Bluetooth antenna design 문서는 안테나 가까이에 금속 물체를 둘 수 없고 detuning을 일으킨다고 설명한다. Silicon Labs 자료도 2.4GHz에서 안테나 주변 1 wavelength가 약 12.5cm이며, 특히 금속은 antenna performance와 spurious emission에 큰 영향을 준다고 설명한다. 따라서 철판 위에 리시버를 놓거나 알루미늄 호일로 감싸 보는 실험에서 신호가 더 튄 것은, 금속 근접 조건이 receiver/antenna 동작에 영향을 준다는 단서로 해석할 수 있다.

### Aluminum foil experiment

알루미늄 호일은 해결책으로 확정하고 적용한 것이 아니라, 금속판 근처에서 신호가 튀는 이유를 확인하기 위해 해 본 troubleshooting 실험이다. 결과적으로 더 심하게 튀었기 때문에, 이 실험은 "금속을 가까이 두면 receiver path가 더 불안정해질 수 있다"는 가설을 강화하는 증거로 보는 것이 자연스럽다.

호일은 전도성 금속이므로 리시버 전체나 안테나 주변을 감싸면 다음 문제가 생길 수 있다.

- 2.4GHz 안테나가 자유공간에 놓인 것이 아니라 금속 근접/감싸임 조건이 되어 detuning된다.
- 금속이 RF 신호를 반사하거나 흡수/차폐해 link margin을 낮춘다.
- 안테나 근처 전계/자계 분포가 바뀌어 수신 감도가 특정 방향에서 급격히 나빠질 수 있다.
- floating foil이면 제대로 접지된 shield처럼 동작하지 않고 예측하기 어려운 기생 도체가 될 수 있다.

즉 이 실험의 의미는 "호일이 최종 해결책이었다"가 아니라, 호일을 가까이 댔을 때 더 나빠졌으므로 금속 근접/안테나 detuning/RF 차폐 영향을 의심하게 만들었다는 점이다. 최종 방향은 receiver antenna를 금속 바닥/ODrive/모터 전원선에서 떨어뜨리고, PWM signal-ground pair를 가깝게 또는 꼬아서 Arduino까지 가져가는 것이다.

### Final mitigation

프로젝트 서술에 넣기 좋은 결론:

> During RC bring-up, the wheel occasionally twitched because the FrSky X8R PWM signal showed sudden spikes on the Arduino serial monitor. Reworking wires and receiver-side connections did not fully solve it. The issue became much worse near a metal base plate. I then intentionally wrapped the receiver with aluminum foil as a troubleshooting experiment to see whether nearby conductive material affected the signal, and the spikes became worse. That result pointed toward a mix of RF antenna detuning and signal-wire EMI pickup. The final practical fix was to keep the receiver/antenna away from metal structures and twist each PWM signal with its ground return, reducing loop area and induced noise.

### Remaining review points

- 최종 배선에서 throttle, steering, engage 각각을 GND와 twist했는지 확인 필요.
- receiver antenna가 metal chassis, ODrive, motor phase wires, battery wires에서 얼마나 떨어졌는지 사진으로 확인하면 좋다.
- PWM spike가 link loss인지, Arduino input edge noise인지, GND bounce인지 완전 분리는 어렵다. 다만 금속/호일 반응과 twisted wiring 해결 결과를 보면 RF + EMI 복합 문제로 정리하는 것이 가장 안전하다.

## 2. ODrive Runaway / Sudden Uncontrolled Motor Behavior

### Symptom

확인된 사용자 설명:

- 잘 가다가 갑자기 한 번씩 모터가 급발진했다.
- 멈추지 않는 현상이 있었다.
- 처음에는 hall sensor 값을 검사했고 정상으로 보였다.
- receiver 신호도 정상으로 보였다.
- 전압 검사 결과 Arduino 출력값도 정상으로 보였다.
- 따라서 ODrive 쪽 문제로 좁혀졌다.
- "ODrive shut off", "급발진" 등을 검색했다.
- ODrive Community 글 "Hoverboard motors not working after release 0.5.1"을 찾았다.
- 글에서는 firmware 0.5.4, 0.5.3, 0.5.2에서 문제가 있었고 0.5.1로 내리면 된다는 기록이 있었다.
- 버전을 downgrade하자 바퀴 급발진 문제가 없어졌다.
- 정확한 내부 원인은 끝까지 찾지 못했다.

### External evidence

- 사용자는 ODrive setup을 ODrive 0.5.6 Getting Started 문서 기준으로 진행했다.
- ODrive 0.5.6 Getting Started는 hoverboard motor를 쓰는 경우 별도 Hoverboard Guide가 있다고 안내한다.
- ODrive Hoverboard Guide는 hall sensor wiring, `ENCODER_MODE_HALL`, `cpr = 90`, hall polarity calibration, hall offset calibration 등 hoverboard motor bring-up 절차를 설명한다.
- 같은 guide는 hoverboard hall signal 신뢰성을 위해 hall sensor pin에 약 22nF filter capacitors를 권장한다. 이는 hall 입력이 noise-sensitive할 수 있음을 보여준다.
- 같은 guide의 Safety 항목은 receiver signal or wire가 끊기면 ODrive PWM input에 timeout 기능이 없어서 마지막 commanded velocity setpoint를 계속 유지할 수 있다고 경고한다. 이 프로젝트는 ODrive 자체 PWM input보다 Arduino가 command를 중간에서 만드는 구조지만, "ODrive가 마지막 명령을 유지할 수 있다"는 safety 관점은 관련 있다.
- ODrive Community 글은 2022년 7월 3일 작성자가 hoverboard motor/hall sensor를 지침대로 설정해도 `EncoderError.ILLEGAL_HALL_STATE`가 발생했고, 0.5.4/0.5.3/0.5.2 rollback에도 실패했지만 0.5.1에서는 hardware 변경 없이 동작했다고 보고한다.

### Local evidence

- `firmware/testers/hall_sensor_test/hall_sensor_test.ino`는 hall sensor A/B/C 상태를 직접 읽고, transition count와 illegal state count를 출력한다. user가 "hall sensor 값 검사 정상"이라고 한 것과 대응되는 테스트 근거로 사용할 수 있다.
- `firmware/testers/motor_current_test/motor_current_test.ino`는 ODrive current command를 직접 보내는 bench test이다. receiver와 balance controller를 빼고 ODrive/motor path만 확인하기 위한 테스트로 볼 수 있다.
- `firmware/testers/odrive_receiver_test/odrive_receiver_test.ino`는 receiver PWM과 ODrive current command를 연결해 보는 integration test이다. 급발진 원인을 receiver 단독, ODrive 단독, integration path로 나눠 확인하려 한 흔적이다.
- 최종 controller는 `ODriveArduino odrive(Serial3)`를 사용하고 `odrive.SetCurrent(0, ...)`, `odrive.SetCurrent(1, ...)`로 전류 명령을 보낸다.
- 최종 controller는 `kMaxAbsCurrent = 8`로 current command를 clamp한다.
- tilt가 30도 이상이면 `tilt_limit_exceeded = true`가 되고, motor active 상태가 꺼지도록 되어 있다.
- legacy controller는 startup에서 바로 closed-loop control로 들어가는 구조였으나, 최종 controller는 engage signal과 tilt limit을 보고 motor activation을 결정한다. 이는 safety 구조가 진화했다는 증거이다.

### Most likely conclusion

가장 조심스럽고 설득력 있는 결론:

> The runaway issue was not proven to be a receiver or Arduino command problem. Hall sensor raw testing looked normal, RC PWM looked normal, and Arduino output checks looked normal, so the fault was isolated toward the ODrive firmware/hoverboard-motor bring-up path. A known ODrive community case reported hoverboard hall calibration issues after firmware 0.5.1, and downgrading firmware removed the runaway symptom in this project. The exact firmware-level root cause remains unconfirmed, but the empirical fix was stable enough to document as a major troubleshooting result.

### Possible root causes, not fully proven

다음은 코드와 외부 사례를 합친 추정이다.

- ODrive firmware version에 따라 hoverboard hall sensor calibration or hall state decoding behavior가 달라졌을 수 있다.
- hall sensor가 raw Arduino test에서는 정상이어도, ODrive 내부 calibration 상태, offset, polarity, CPR, bandwidth, current control bandwidth 설정과 함께 쓰일 때 문제가 생겼을 수 있다.
- ODrive serial command/feedback timing이 꼬이거나, controller state가 closed loop에서 예상치 못하게 남아 있었을 수 있다.
- ODrive가 마지막 command/current state를 유지하는 safety issue가 일부 상황에서 "멈추지 않음"처럼 보였을 수 있다.
- motor phase/hall wiring 조합이 특정 firmware calibration sequence에서만 취약했을 수 있다.

이 문서에서는 "firmware downgrade로 해결됐으니 ODrive bug가 확정"이라고 단정하지 않는 편이 좋다. 더 정확한 표현은 "ODrive firmware/hall calibration compatibility issue로 격리되었고, downgrade로 해결되었다"이다.

### Remaining review points

- 최종적으로 사용한 ODrive firmware version을 명확히 확인해야 한다. 사용자 설명상 0.5.1 downgrade가 핵심 후보이다.
- ODrive error dump 로그가 남아 있다면 `dump_errors(odrv0)` 결과를 추가하면 원인 신뢰도가 크게 올라간다.
- ODrive board revision이 3.6인지, 커뮤니티 글의 v3.5 사례와 차이가 있었는지 별도 표기하면 좋다.
- 최종 ODrive 설정값: pole pairs, hall CPR, current limit, current control bandwidth, encoder bandwidth, brake resistor/negative current 설정을 별도 appendix로 남기면 재현성이 올라간다.

## 3. Hall Sensor And Motor Bring-Up

### Confirmed work

- Hall sensor tester가 존재한다.
- Motor current tester가 존재한다.
- ODrive + receiver integration tester가 존재한다.
- ODrive hoverboard motor setup은 공식 문서 기준으로 진행되었다.

### Why this mattered

홀센서 BLDC 허브모터는 "전원 연결 후 그냥 도는 모터"가 아니라, phase wiring, hall wiring, hall state sequence, pole pair, CPR, calibration, current control이 모두 맞아야 한다. ODrive Hoverboard Guide는 표준 hoverboard hub motor가 15 pole pairs이고 hall feedback은 pole pair당 6 state라서 CPR 90을 설정한다고 설명한다.

### Problems implied by the test files

- Hall sensor A/B/C line이 잘못 연결되면 illegal state or wrong sequence가 나올 수 있다.
- ODrive calibration은 motor가 자유롭게 움직일 수 있어야 한다.
- Hall signal에는 filtering capacitor가 권장될 정도로 noise sensitivity가 있다.
- 직접 current command를 주는 bench test가 있다는 것은 full balance loop에 들어가기 전에 motor response를 따로 확인해야 했다는 뜻이다.

## 4. ODrive Serial Feedback And Encoder Spike Handling

### Local evidence

- `archive/legacy_firmware/legacy_balance_controller.ino`는 ODrive에서 position과 velocity를 읽을 때 `Serial2` buffer를 비우고, 여러 값을 `parseFloat()`로 읽는다.
- 같은 파일에는 `r axis1.encoder.vel_estimate`가 한 번 더 요청되며 "문제 해결용 추가 명령어"라는 주석이 있다. 이는 ODrive serial response alignment or parsing 문제를 해결하려 한 흔적이다.
- legacy controller는 raw position과 velocity 모두에 adaptive threshold와 median smoothing을 적용한다.
- 최종 controller도 encoder 읽기 전후로 `Serial3` buffer를 비운다.
- 최종 controller에는 `filterEncoderData()` 함수가 있어 abs(new_value) > 100이면 이전 값을 반환하는 outlier rejection 로직이 있다.
- 다만 최종 main loop에서 이 함수가 실제로 호출되는지는 현재 코드만 보면 명확하지 않다. 또한 `kAlpha_3 = 1`이라 `filtered_phi_dot_*`는 이름과 달리 사실상 no filtering이다.

### Inference

ODrive에서 읽는 encoder/speed feedback이 한때 튀거나 serial parsing이 어긋나는 문제가 있었을 가능성이 높다. legacy code의 median/adaptive threshold와 최종 code의 serial flush/outlier function은 그 흔적이다.

### Impact

self-balancing robot에서 wheel velocity feedback이 순간적으로 튀면 speed error, integral term, balance torque adjustment가 모두 흔들릴 수 있다. 실제 급발진 문제와 같은 root cause라고 단정할 수는 없지만, "바퀴가 튄다" 또는 "갑자기 이상한 current command가 나간다"는 증상에 영향을 줄 수 있는 경로이다.

## 5. RC Neutral Drift, Deadband, And Engage Safety

### Local evidence

- receiver neutral offset이 1500이 아니라 steering 1492, throttle 1488로 따로 잡혀 있다.
- `kControlThreshold = 50` deadband가 있다.
- throttle/steering 값을 normalized input으로 바꾸기 전에 deadband를 적용한다.
- engage PWM은 low-pass filter와 persistence counter를 거친다.
- legacy IMU/ROS firmware에도 noisy PWM 때문에 false disengagement를 막는 persistence filtering 주석이 있다.

### Inference

RC receiver는 중립에서도 흔들렸고, 이를 그대로 motor command에 연결하면 robot이 가만히 있어야 할 때 미세하게 움직이거나, engage 상태가 튀거나, wheel angle correction이 잘못 들어갔을 가능성이 있다.

### What was fixed

- neutral offset을 실측값으로 조정했다.
- 작은 stick movement or noise는 deadband로 무시했다.
- engage는 순간값이 아니라 몇 cycle 동안의 persistence로 판정했다.
- PWM 값을 low-pass filter로 부드럽게 했다.

## 6. Balance Tuning Sensitivity And Fall Risk

### Local evidence

- `docs/build_story.md`는 full body assembly 이후 tethered practice로 파라미터를 조정했다고 설명한다.
- `docs/hardware_gallery.md`에는 tethered driving practice가 safety step이었다고 적혀 있다.
- 최종 controller에는 tilt cutoff 30도, max current 8A, speed/current clamp가 있다.
- legacy experimental firmware는 max current 4A로 더 보수적인 제한을 쓴다.
- ROS PID experiments는 여러 PID variant와 auto-tuning 파일을 남긴다.
- `pid_bayes_optimizer.py`는 robot falling을 감지하면 penalty를 주고 simulation reset을 수행한다.

### Inference

balance control은 작은 gain 변화, IMU angle offset, wheel speed feedback, current limit에 매우 민감했다. 그래서 처음부터 무제한 current나 untethered test를 하지 않고, tethered practice와 current clamp를 두고 gain을 단계적으로 조정한 것으로 보인다.

### Project narrative

이 프로젝트의 중요한 성과는 "한 번에 완성"이 아니라, 다음의 안전 단계가 쌓인 것이다.

- IMU 단독 확인
- receiver PWM 확인
- hall sensor 확인
- ODrive motor current bench test
- ODrive + receiver integration test
- tethered balance practice
- RC driving while balancing

## 7. BNO055 IMU Calibration And Angle Offset

### Local evidence

- BNO055 initialization 실패 시 controller가 halt한다.
- EEPROM에 BNO055 calibration offset을 저장/로드하는 함수가 있다.
- calibration status를 출력하고, magnetometer calibration을 위해 figure-8 movement가 필요하다는 메시지가 있다.
- `imu_angle_offset = 3.5`가 설정되어 있다.

### Inference

IMU calibration과 mounting angle offset이 balance quality에 직접 영향을 줬다. BNO055가 같은 자세에서도 calibration 상태나 장착 각도에 따라 pitch값을 다르게 줄 수 있기 때문에, offset과 calibration save/load가 필요했을 것이다.

### Risk

IMU calibration이 덜 된 상태에서 balance loop를 켜면 robot은 실제 upright가 아닌 각도를 upright로 오인할 수 있다. 이것은 slow drift, wheel creeping, sudden correction, fall risk로 이어질 수 있다.

## 8. Power, Brake, And Safety Considerations

### Local evidence

- hardware docs는 36V battery, 36V -> 19V, 19V -> 5V conversion chain을 설명한다.
- ODrive 공식 문서는 brake resistor가 없으면 deceleration energy가 power supply로 돌아가 bus voltage가 올라갈 수 있고, overvoltage protection이 trip하면 motor가 free-spin할 수 있다고 경고한다.
- ODrive 공식 문서는 encoder/motor mechanical slip이 disastrous oscillations or runaway를 만들 수 있다고 경고한다.
- 최종 firmware는 tilt cutoff와 current clamp가 있다.

### Inference

ODrive 기반 밸런싱 로봇은 power path와 safety path가 매우 중요했다. 전원, GND, brake/regen behavior, current limit, motor activation state가 잘못되면 "제어 알고리즘 문제"처럼 보이는 위험 증상이 생길 수 있다.

## 9. ROS/Gazebo Command Architecture Problem

### Confirmed design issue

일반 differential drive robot은 `/cmd_vel`을 바로 받아도 된다. 하지만 self-balancing robot은 `/cmd_vel`을 곧장 wheel command로 넣으면 안 된다. balance controller가 body angle, angular velocity, wheel velocity, speed target을 함께 처리해야 한다.

### Local evidence

- `docs/software_architecture.md`는 `move_base` or teleop이 직접 balancing robot을 drive하지 않고 `/before_vel`에 high-level command를 보내고, balancing controller가 final `/cmd_vel`로 변환한다고 설명한다.
- `ros_ws/src/navigation/launch/move_base.launch`는 `cmd_vel_topic` default를 `/before_vel`로 둔다.
- 여러 PID scripts는 `/before_vel` subscriber와 `/cmd_vel` publisher를 가진다.

### Inference

초기에는 navigation stack의 `/cmd_vel`과 balance controller의 `/cmd_vel`이 충돌하거나 역할이 섞였을 가능성이 있다. `/before_vel` 설계는 이 문제를 해결하기 위한 architecture fix이다.

## 10. Gazebo Reset, PID Tuning, And Simulation Instability

### Local evidence

- `pid_control_before_vel_lidar_pid_tuning.py`는 ROS time이 뒤로 가면 simulation reset으로 감지하고 PID state를 reinitialize한다.
- `gazebo_reset_world.py`, `pid_loop_test_runner.py`, `pid_bayes_optimizer.py`는 Gazebo pause/reset/unpause service를 사용한다.
- `pid_bayes_optimizer.py`는 robot falling을 감지하고 penalty를 준다.
- PID experiments가 `archive/legacy_code/pid_experiments/`에 여러 variant로 남아 있다.

### Inference

simulation에서도 robot이 자주 넘어지거나 reset 후 PID integral/previous error가 남아 이상 동작했을 수 있다. reset detection과 PID state clear는 이 문제를 줄이기 위한 코드이다.

## 11. Physical SLAM/Navigation Scope Limitation

### Confirmed from docs

- `README.md`와 `docs/results_and_limitations.md`는 real-world ROS SLAM/navigation integration을 partial로 명시한다.
- physical balancing and RC driving은 verified outcome이다.
- simulation navigation은 completed로 정리되어 있다.
- physical autonomous navigation은 final completed claim으로 두지 않는다.

### Local evidence

- `archive/legacy_code/real_world_integration/robot_slam.launch`는 Orbbec Gemini 330 launch와 RTAB-Map launch를 포함한다.
- `ros_ws/src/robot_ability/launch/robot_slam_depth.launch`와 navigation depth files에는 historical `/d435/...` topic이 남아 있다.
- `docs/hardware_bom.md`는 D435 names가 historical simulation/placeholder이고 실제 physical camera는 Orbbec Gemini 330이라고 명시한다.

### Inference

카메라/SLAM/navigation은 시도와 integration evidence가 있지만, physical robot에서 end-to-end autonomous navigation까지 검증된 상태는 아니었다. 따라서 포트폴리오에서는 "physical robot hardware includes Orbbec Gemini 330 and real-world integration was attempted" 정도가 안전하다.

## 12. Repository Reproducibility And Evidence Issues

### Confirmed issues

- `docs/setup.md`는 `rtabmap_ros`, `realsense_ros_gazebo`, `rosserial` 등 third-party dependencies가 vendored 되어 있지 않다고 설명한다.
- `docs/results_and_limitations.md`는 third-party dependency versions를 더 tight하게 pin해야 한다고 적는다.
- `ros_ws/src/navigation/maps/*.yaml`에는 `/home/yggulbi/...` absolute image path가 있다. 다른 환경에서 바로 map load가 깨질 수 있다.
- 여러 `package.xml`에는 `<license>TODO</license>`가 남아 있다.
- 일부 launch files는 `robot_gazebo` 같은 old package name을 참조하지만, 현재 package directory는 `balance_robot_gazebo` 계열이다. 이는 historical naming mismatch risk이다.
- original MP4s는 repo에 모두 들어 있지 않고, curated GIF/image와 external-only evidence로 정리되어 있다.
- hardware annotated image는 오해 위험 때문에 raw photo와 conservative redraw 중심으로 바뀌었다.

### Meaning

이 repo는 "완전 재현 가능한 lab notebook"이라기보다 "검증 가능한 포트폴리오 정리본"이다. 따라서 docs에서 verified, partial, recovered, external-only evidence를 구분하는 현재 방향이 맞다.

## 13. Image/Media Rendering Risk

현재 docs는 상대 경로로 image를 참조한다. GitHub에서 이미지가 안 뜨는 문제는 보통 다음 중 하나이다.

- 실제 파일 경로와 Markdown path의 대소문자/폴더명이 다름
- Git LFS pointer만 있고 실제 asset이 push되지 않음
- 파일이 너무 크거나 GitHub preview가 지원하지 않는 형식
- Markdown이 raw document path를 가리키는데 public repo에는 해당 파일이 없음
- SVG 내부에서 외부/local asset을 참조함
- branch에 media 파일이 commit되지 않았거나 push되지 않음

이미 현 repo 문서는 `media/hardware`, `media/diagrams`, `media/process`, `media/demos`, `media/hero` 등으로 정리되어 있다. 이미지 깨짐이 계속 보이면 `rg -n "!\\[|<img" README.md docs`로 모든 이미지 링크를 뽑고, 각 path가 실제 존재하는지 점검하는 것이 좋다.

## 14. Suggested Final Story For Reviewers

검수 후 README나 발표 자료에 넣기 좋은 짧은 narrative:

> The hardest part of the physical robot was not writing the balance equation once, but making the full hardware loop trustworthy. The FrSky receiver initially produced occasional PWM spikes that made the wheels twitch. Serial inspection showed the receiver pulse width jumping, and the issue became much worse near a metal base plate. To test whether nearby conductive material was part of the problem, I intentionally wrapped the receiver with aluminum foil; the spikes became worse, so I treated the issue as an RF/signal-integrity problem. The receiver was moved away from metal, and each PWM signal was routed with a twisted ground return to reduce loop area and induced noise.  
>
> A second major issue was intermittent uncontrolled motor behavior through the ODrive path. Hall sensors, receiver PWM, and Arduino-side outputs were checked first, which isolated the issue toward the ODrive hoverboard motor firmware/calibration path. I found an ODrive community report that hoverboard hall calibration failed on firmware after 0.5.1 and worked after downgrading. After applying the downgrade, the runaway symptom disappeared. I did not fully prove the firmware-level root cause, so I document it as an empirically solved ODrive firmware/hoverboard hall compatibility issue rather than over-claiming a definitive bug.

## 15. Questions To Verify Before Final Publication

- 최종 ODrive firmware version은 정확히 몇인가? 0.5.1인지, 다른 downgrade version인지 확인 필요.
- ODrive board revision은 3.6으로 최종 확정인가?
- ODrive `dump_errors(odrv0)` 로그가 남아 있는가?
- receiver PWM spike가 throttle/steering/engage 중 어느 channel에서 가장 심했는가?
- 최종 twisted wiring은 signal-ground pair별로 했는가, 아니면 여러 signal을 한 번에 꼬았는가?
- receiver antenna는 최종적으로 금속 chassis에서 어느 정도 떨어져 있었는가?
- ODrive hoverboard setup값: pole pairs, CPR, current limit, current control bandwidth, encoder bandwidth, brake resistor setting이 남아 있는가?
- 최종 controller에서 `filterEncoderData()`를 실제로 썼는가? 현재 코드만 보면 함수는 있지만 main loop 호출은 확인되지 않는다.
- physical navigation demo evidence가 있다면 `partial` claim을 더 강하게 바꿀 수 있는가? 없으면 현재처럼 partial 유지가 안전하다.
