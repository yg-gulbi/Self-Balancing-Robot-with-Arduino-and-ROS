# Navigation 명령을 그대로 보내지 않고 밸런스 제어기를 통과시켰다

[English](../../../troubleshooting/supporting/03-ros-command-path.md) | [전체 트러블슈팅](../../troubleshooting.md)

> `move_base` 출력은 이동 의도로 받고, IMU·Odom을 사용하는 밸런스 제어기가 최종 `/cmd_vel`을 출력하도록 시뮬레이션 경로를 구성했다.

## 이동 요청과 자세 복원이 같은 출력 경로를 사용한다

Navigation은 목적지로 가기 위한 이동 속도를 요청한다. 하지만 2륜 밸런스 로봇의 바퀴는 이동과 몸체 복원을 동시에 수행한다. 따라서 상위 속도 명령을 베이스에 직접 전달하면 자세 제어가 계산해야 할 출력과 충돌할 수 있었다.

이 항목은 독립적인 고장 로그보다, 시뮬레이션에서 명령 책임을 나눈 설계 문제에 해당한다.

## 먼저 Navigation의 출력 목적지를 바꿨다

[move_base.launch](../../../../ros_ws/src/navigation/launch/move_base.launch)는 목적지 토픽의 기본값을 `/before_vel`로 두고 출력 remap을 적용한다.

```xml
<arg name="cmd_vel_topic" default="/before_vel" />
```

```xml
<remap from="cmd_vel" to="$(arg cmd_vel_topic)"/>
```

Navigation이 계산한 명령은 `/before_vel`에 도착한다. 이 토픽은 바퀴에 바로 전달할 최종 출력이 아니라 밸런스 제어기에 주는 목표 속도·회전 입력이다.

## 밸런스 제어기에서 어떤 정보가 추가되는가

[LiDAR 시뮬레이션 제어기](../../../../ros_ws/src/balance_robot_control/src/controllers/pid_control_before_vel_lidar.py)는 입력과 상태를 다음과 같이 나눈다.

| 입력·출력 | 코드에서의 역할 |
| --- | --- |
| `/before_vel` | 목표 선속도·회전 입력 설정 |
| `/odom` | 현재 선속도 갱신 |
| `/imu` | pitch·자세 변화 계산, 제어 출력 갱신 |
| `/cmd_vel` | 계산된 베이스 명령 발행 |

속도 오차를 목표 기울기로 변환하고 현재 pitch와 비교한다.

```python
speed_error = self.setpoint_speed - self.current_speed
self.setpoint_angle = self.Kp_speed * speed_error
angle_error = self.setpoint_angle - current_pitch_angle
output_angle = (self.Kp_angle * angle_error + self.Kd_angle * pitch_rate)
```

이 과정을 통해 상위 명령에 현재 이동·자세 상태가 반영된다. LiDAR 버전의 제어 출력 계산은 IMU 콜백에서 실행된다. 따라서 `rospy.Rate(200)` 선언만으로 전체 제어가 측정된 200 Hz라고 결론내리지는 않는다.

## 토픽 이름보다 실제 발행·구독 관계가 중요했다

문서상의 경로는 `move_base → /before_vel → balance_robot_control → /cmd_vel`이다. launch remap과 제어기의 subscriber·publisher를 함께 읽어야 이 관계를 확인할 수 있다.

![시뮬레이션 Navigation 화면](../../../../media/process/simulation_depth_navigation_views.png)

*공개된 depth 모델 시뮬레이션 화면. 위 코드 발췌는 LiDAR 버전이며, 영상·이미지는 depth 버전의 워크플로 결과다.*

두 모델의 설정이 완전히 같다는 의미는 아니지만, 상위 이동 의도와 최종 밸런스 출력을 구분하는 구조를 공유한다. 실물 쪽에서는 Arduino가 ODrive 전류 요청을 계산한다. 구조의 공통점과 실물 자율주행 완료 여부는 별개의 사실이다.

## 면접에서 확인할 수 있는 근거

**이 사례가 보여주는 작업:** 상위 계획과 하위 제어의 책임 분리, ROS 토픽·launch 구성, 시뮬레이션 검증 범위 구분.

- [move_base.launch](../../../../ros_ws/src/navigation/launch/move_base.launch): Navigation 출력 remap.
- [LiDAR 제어 코드](../../../../ros_ws/src/balance_robot_control/src/controllers/pid_control_before_vel_lidar.py): 입력·피드백·최종 출력 연결.
- [제어 패키지 설명](../../../../ros_ws/src/balance_robot_control/README.md): 모델별 구조와 튜닝 경로.
- [시뮬레이션 영상](../../../../media/process/simulation_depth_navigation_demo.webm): depth 워크플로 동작.
- [Sim2Real 범위](../../sim2real.md): 실물 자율주행은 연동 실험 단계.

launch와 코드로 경로를 검토할 수 있고, 실행 중에는 `rostopic info /before_vel`, `rostopic info /cmd_vel`로 실제 publisher·subscriber를 확인할 수 있다. 공개 데모는 시뮬레이션 결과이며 실물의 완성된 자율주행 증거로 사용하지 않는다.
