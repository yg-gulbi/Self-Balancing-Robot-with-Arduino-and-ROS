# ROS 주행 명령 경로와 밸런스 제어 권한

[English](../../../troubleshooting/supporting/03-ros-command-path.md) | [트러블슈팅 목차](../../troubleshooting.md)

**분류:** 보조 이슈 · 기존 문서와 코드에서 재구성한 기록

## 1. 문제정의

Navigation은 이동 속도를 요청하지만 2륜 밸런스 로봇은 계속 자세를 안정화해야 한다. 상위 속도 명령이 베이스로 바로 전달되면 자세와 이동 요구를 조정하는 계층을 우회한다. 이 항목은 독립적인 하드웨어 고장보다 제어 구조상의 문제를 다룬다.

## 2. 해결방법 안들 설정

이동 의도와 최종 베이스 명령을 분리한다. Teleop과 Navigation은 의도 토픽에 입력하고 밸런스 제어기가 IMU·Odom 피드백과 결합한 뒤 최종 명령을 출력하도록 구성한다. 실물 연동을 설명할 때도 이 제어 권한 경계를 유지한다.

## 3. 해결방법 실행

시뮬레이션 패키지는 `/before_vel`, `/imu`, `/odom`을 구독하고 `/cmd_vel`을 출력한다. 문서화된 주행 경로는 `move_base → /before_vel → balance_robot_control → /cmd_vel`이며 제어 코드와 launch 설정에서 확인할 수 있다. 실물에서는 Arduino의 자세·안전 루프가 ODrive 전류 명령을 출력한다. 설계 원칙이 같다는 사실이 실물 자율주행 완성을 의미하지는 않는다.

## 4. 경험의 재해석

토픽 분리는 명령 책임을 드러내지만 이름만으로 권한이 보장되지는 않는다. 실제 launch remap과 실행 중 publisher가 밸런스 제어기를 우회하지 않는지 확인해야 한다.

## 5. 정리

결과：이동 의도–밸런스 제어–최종 출력 경로는 시뮬레이션에서 문서화·구현되어 있으며 실물 자율주행은 연동 실험 단계다. 패키지·Navigation launch·Sim2Real 한계를 확인한다. 실행 중에는 `rostopic info /before_vel`, `rostopic info /cmd_vel`로 발행·구독 관계를 확인하고 이동·정지 시 의도 속도, pitch, 출력 명령을 비교한다.

### 검증 근거

- [Balance control package](../../../../ros_ws/src/balance_robot_control/README.md)
- [Navigation package](../../../../ros_ws/src/navigation)
- [Sim2Real boundaries](../../../../docs/sim2real.md)
