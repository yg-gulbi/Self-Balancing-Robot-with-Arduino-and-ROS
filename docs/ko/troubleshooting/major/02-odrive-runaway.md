# ODrive 폭주와 비정상 모터 동작

[English](../../../troubleshooting/major/02-odrive-runaway.md) | [트러블슈팅 목차](../../troubleshooting.md)

**분류:** 중요 이슈 · 기존 문서와 코드에서 재구성한 기록

## 1. 문제정의

간헐적인 급가속이나 정상적으로 멈추지 않는 현상이 발생했다. RC 입력, 홀센서 피드백, Arduino 명령 전달, ODrive 설정·펌웨어 중 어느 경로에서 문제가 생기는지 분리해야 했다.

## 2. 해결방법 안들 설정

홀센서 상태 시험, 수신기 PWM 시험, 수신기–ODrive 통합 시험, 전류 명령 단독 시험으로 경로를 나눈다. 원인 범위를 좁힌 뒤 펌웨어 호환성을 검토한다. 원인 조사와 별개로 모터 활성 상태와 전류 명령을 제한하는 방안도 검토한다.

## 3. 해결방법 실행

기존 기록에서는 홀센서 시험으로 원인을 특정하지 못했고, 펌웨어 다운그레이드 이후 폭주 증상이 사라졌다고 보고한다. 정확한 변경 대상 버전, 버전별 반복 시험표, 내부 결함 추적 자료는 없다. 최종 제어기는 `IDLE`과 `CLOSED_LOOP_CONTROL`을 전환하고 기울기·활성화 신호로 구동 여부를 판단하며, 송신 전류 명령을 ±8 A로 제한한다. 이 제한은 Arduino가 보내는 명령에 적용되며 ODrive 내부의 모든 고장을 차단한다는 의미는 아니다.

## 4. 경험의 재해석

증상을 제거한 변경은 유효한 단서지만 정확한 원인 규명과는 구분해야 한다. 따라서 ‘펌웨어 다운그레이드 후 경험적으로 해결’로 표현한다. 경로별 단독 시험은 원인 범위를 좁히고, 상태 감독과 명령 제한은 비정상 제어 상태의 영향을 줄이는 역할을 한다.

## 5. 정리

결과：다운그레이드 이후 증상 소멸은 기존 기록에 보고되어 있고, 활성화 조건과 전류 명령 제한은 코드에서 확인할 수 있다. 아래 4개 테스터와 `EngageMotors()`, `ApplyMotorCommands()`를 검토한다. 추가 검증에는 변경 전후 펌웨어 버전, 동일 시험 조건, 재발 횟수, ODrive 오류 상태 기록이 필요하다.

### 검증 근거

- [Hall tester](../../../../firmware/testers/hall_sensor_test/hall_sensor_test.ino)
- [Receiver tester](../../../../firmware/testers/receiver_pwm_test/receiver_pwm_test.ino)
- [ODrive receiver tester](../../../../firmware/testers/odrive_receiver_test/odrive_receiver_test.ino)
- [Current tester](../../../../firmware/testers/motor_current_test/motor_current_test.ino)
- [Physical controller](../../../../firmware/physical_balance_controller/physical_balance_controller.ino)
