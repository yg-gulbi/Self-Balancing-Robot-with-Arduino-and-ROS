# 모터가 갑자기 가속했다: ODrive 경로를 나누어 조사하기

[English](../../../troubleshooting/major/02-odrive-runaway.md) | [전체 트러블슈팅](../../troubleshooting.md)

> 간헐적인 비정상 가속을 RC·홀센서·명령 전달·모터 제어 경로로 나누어 조사했다. 기존 기록에서는 펌웨어 다운그레이드 이후 증상이 사라졌다고 보고하며, 저장소에는 각 경로를 확인하는 테스터와 최종 상태·전류 제한 코드가 남아 있다.

## 같은 바퀴 움직임이라도 RC 노이즈와는 구분해야 했다

ODrive를 통한 구동 중 갑자기 가속하거나 정상적으로 멈추지 않는 현상이 있었다. RC 신호 튐도 존재했지만, 그것만으로 모든 비정상 동작을 설명할 수는 없었다. 자세 제어, 입력 해석, 홀센서 상태, ODrive 설정이 동시에 개입하는 전체 로봇에서 곧바로 게인을 바꾸면 원인 후보가 더 섞인다.

따라서 조사 질문을 “로봇이 왜 폭주하는가”에서 **“어느 경로까지는 정상이라고 확인할 수 있는가”**로 바꿨다.

## 전체 로봇을 네 가지 확인 경로로 나눴다

저장소의 테스터는 각기 다른 변수를 남기고 나머지 경로를 덜어낸다.

| 확인 경로 | 확인 가능한 항목 | 분리하려는 원인 후보 |
| --- | --- | --- |
| 홀센서 단독 | A/B/C 상태, 전이 횟수, illegal 상태 카운터 | 센서 연결·상태 신호 문제 |
| 수신기 단독 | throttle·steering·engage 펄스폭 | RC 입력 자체의 문제 |
| 수신기–ODrive 통합 | PWM, 좌우 전류 요청, active 상태 | 입력을 전류로 변환하는 경로 |
| 전류 명령 단독 | axis별 전류 요청, 정지·IDLE 요청 | RC·IMU를 제외한 모터 명령 경로 |

홀센서 테스터는 A0/A1/A2를 읽고, 세 신호의 합이 0 또는 3인 상태를 illegal로 분류한다. 출력에는 다음 항목이 있다.

```text
hall_a    hall_b    hall_c    state    transitions    illegal
```

이 카운터는 상태 이상을 볼 수 있는 수단이지 “실험에서 illegal이 0회였다”는 결과표가 아니다. 기존 기록에서는 홀센서 확인만으로 폭주 원인이 드러나지 않아 다른 경로 조사를 계속했다고 설명한다.

## 입력을 제거한 시험이 판단을 좁혀주었다

[전류 명령 테스터](../../../../firmware/testers/motor_current_test/motor_current_test.ino)는 RC와 IMU를 사용하지 않는다. Arduino 시리얼 명령으로 한 축 또는 양쪽 축에 전류를 요청하고, 0 A 요청과 IDLE 전환을 구분한다.

| 테스터 명령 | 코드에서 하는 일 |
| --- | --- |
| `m <axis> <amps>` | 지정 축에 제한된 전류 요청 |
| `b <amps>` | 양쪽 축에 전류 요청 |
| `s` | 양쪽 전류를 0 A로 요청 |
| `d` | 0 A 요청 후 양쪽 축에 IDLE 요청 |

이 구조 덕분에 “수신기 입력이 잘못되었는가”와 “정상적인 전류 요청에도 모터 경로가 비정상적인가”를 다른 시험으로 다룰 수 있었다. 저장소는 원래 프로젝트 자료를 복원·정리한 형태이므로, 현재 테스터의 존재만으로 당시 모든 시험의 수치 결과까지 복원되었다고 보지는 않는다.

기존 조사 기록의 결과는 **펌웨어 다운그레이드 이후 폭주 증상이 사라졌다**는 것이다. 이 결과는 ODrive 펌웨어·홀센서 구동 호환성 경로를 의심할 이유를 주었다. 다만 정확한 변경 전후 버전표와 내부 결함 추적 기록이 없어, 특정 버그를 완전히 규명했다고 결론내리지는 않았다.

## 증상 해소 뒤에도 모터 상태를 명시적으로 다뤘다

최종 코드에는 구동 상태를 전환하는 `EngageMotors()`와 전류 명령을 제한하는 `ApplyMotorCommands()`가 있다.

```cpp
float current_command_0 = constrain(torque_input_0, -kMaxAbsCurrent, kMaxAbsCurrent);
float current_command_1 = constrain(torque_input_1, -kMaxAbsCurrent, kMaxAbsCurrent);
```

`kMaxAbsCurrent=8.0`이므로 Arduino가 보내는 전류 요청은 ±8 A로 제한한다. engage 신호와 기울기 조건은 ODrive 축의 `IDLE ↔ CLOSED_LOOP_CONTROL` 전환 요청에 사용한다.

여기서 0 A 요청, 축의 IDLE 요청, 전원 차단은 서로 다른 동작이다. 이 코드가 보장하는 것은 요청값 제한과 상태 전환 로직의 존재이며, 실제 전류 측정이나 모든 ODrive 내부 고장에 대한 독립 보호를 뜻하지 않는다.

## 해결 결과와 원인 규명을 구분하게 되었다

이번 문제는 펌웨어 변경이 증상에 영향을 주었다는 경험적 결과를 남겼다. 동시에 RC·홀센서·직접 전류 경로를 나누면 다음 조사에서 같은 현상을 더 좁은 범위로 재현할 수 있다는 방법도 남겼다.

면접에서 설명할 핵심은 “다운그레이드로 해결했다” 한 문장이 아니라, **입력·피드백·명령 경로를 구분하고 무엇을 확인할 수 있는지 정한 과정**이다.

## 면접에서 확인할 수 있는 근거

**이 사례가 보여주는 작업:** 시스템 원인 분리, 단독 시험 구성, 증상 해소와 원인 확정의 구분.

- [hall_sensor_test.ino](../../../../firmware/testers/hall_sensor_test/hall_sensor_test.ino): 상태·전이·illegal 카운터.
- [receiver_pwm_test.ino](../../../../firmware/testers/receiver_pwm_test/receiver_pwm_test.ino): 입력 펄스폭 관찰.
- [odrive_receiver_test.ino](../../../../firmware/testers/odrive_receiver_test/odrive_receiver_test.ino): PWM과 전류 요청의 연결.
- [motor_current_test.ino](../../../../firmware/testers/motor_current_test/motor_current_test.ino): RC·IMU 없이 전류·상태 요청.
- [최종 제어기](../../../../firmware/physical_balance_controller/physical_balance_controller.ino): 전류 제한과 상태 전환.

다운그레이드 후 증상 소멸은 기존 기록에 근거한다. 정확한 버전 조합, 동일 조건의 반복 횟수, ODrive 오류 상태 로그가 추가되면 펌웨어 변경과 결과의 연결을 더 강하게 검증할 수 있다.
