# Arduino-ROS 셀프 밸런싱 로봇

[English](README.md) | 한국어

이 저장소는 Arduino와 ROS/Gazebo를 함께 사용한 셀프 밸런싱 로봇 프로젝트입니다. 실제 로봇은 Arduino에서 제어했고, ROS/Gazebo는 시뮬레이션, 튜닝, SLAM, 내비게이션 실험에 사용했습니다.

가장 중요한 결과는 실제 2륜 로봇이 Arduino 제어로 약 1시간 동안 균형을 유지했고, 10 m 복도 주행과 실내 장애물 코스 주행을 수행했다는 점입니다. ROS 쪽에서는 Gazebo 제어, 내비게이션, SLAM 지도 생성, PID 튜닝, Arduino-to-ROS 브리지 흐름을 구성했습니다.

<p align="center">
  <img src="media/hero/physical_balance_hallway.gif" alt="Physical hallway balancing demo" width="720">
</p>

## 결과 요약

| 항목 | 결과 |
| --- | --- |
| 실제 균형 유지 시간 | 약 1시간 standing balance |
| 실제 주행 | 10 m 복도 주행 및 장애물 코스 주행 |
| 실제 안전 한계 | 보호 브래킷이 약 `28 deg`에서 바닥에 닿기 때문에, 모터 명령 차단 각도를 `30 deg`로 설정 |
| ODrive 전류 제한 | 펌웨어에서 `+/-8 A`로 current command clamp |
| RC 신호 처리 | `+/-50 us` deadband, throttle/steering filter alpha `0.4`, engage filter alpha `0.02` |
| Arduino-to-ROS 브리지 | `/imu`, `/odom`, `cmd_vel`을 `rostopic echo`로 확인 |
| ROS/Gazebo 내비게이션 | ROS 내비게이션 명령 경로를 통해 로봇 이동 확인 |
| ROS/Gazebo SLAM | SLAM workflow에서 지도 생성 확인 |

## 핵심 결과

<p align="center">
  <img src="media/diagrams/physical_balance_control_loop.png" alt="Physical balance control loop" width="860">
</p>

Arduino는 실시간 balance loop를 로컬에서 닫습니다. RC 입력, IMU tilt/yaw feedback, wheel-speed feedback, safety check, ODrive current command가 하나의 제어 경로 안에서 결합됩니다.

| 제어 계층 | 역할 |
| --- | --- |
| Balance | Body angle과 angular velocity로 주된 안정화 current term을 만듭니다. |
| Motion | RC throttle을 target speed로 바꾸고, 로봇이 균형을 유지한 채 움직일 수 있도록 balance point를 이동시킵니다. |
| Safety | PWM filtering, engage persistence, tilt cutoff, current limiting으로 위험한 상태가 모터까지 전달되지 않게 합니다. |

먼저 [physical balance control algorithm](firmware/physical_balance_controller/control_algorithm.md)과 [troubleshooting summary](docs/ko/troubleshooting.md)를 보면 좋습니다.

## 프로젝트 범위

| 영역 | 상태 | 볼 곳 |
| --- | --- | --- |
| 실제 self-balancing 및 RC 주행 | 완료 | [physical_balance_controller.ino](firmware/physical_balance_controller/physical_balance_controller.ino), [hallway demo](media/hero/physical_balance_hallway.gif) |
| ROS/Gazebo balance simulation | 완료 | [balance_robot_control](ros_ws/src/balance_robot_control), [balance_robot_gazebo](ros_ws/src/balance_robot_gazebo) |
| Simulation navigation pipeline | 완료 | [navigation](ros_ws/src/navigation), [balance_robot_workflows](ros_ws/src/balance_robot_workflows) |
| Simulation SLAM/navigation workflow | 시뮬레이션에서 완료 | [balance_robot_workflows](ros_ws/src/balance_robot_workflows), [결과와 한계](docs/ko/results-and-limitations.md) |
| Arduino-to-ROS bridge tests | 완료 | [rc_to_ros_cmd_vel_bridge.ino](firmware/testers/rc_to_ros_cmd_vel_bridge.ino), [physical_balance_controller_ros.ino](firmware/physical_balance_controller_ros/physical_balance_controller_ros.ino) |
| Real-world ROS SLAM/navigation | 통합 실험 | [real-world integration archive](archive/ros_experiments/real_world_integration), [결과와 한계](docs/ko/results-and-limitations.md) |

## 시스템 한눈에 보기

<table>
  <tr>
    <td width="50%">
      <img src="media/diagrams/signal_control_diagram.png" alt="Signal / Control Diagram" width="100%">
    </td>
    <td width="50%">
      <img src="media/diagrams/wiring_diagram.png" alt="Wiring Diagram" width="100%">
    </td>
  </tr>
  <tr>
    <td valign="top">
      <strong>제어 책임</strong><br>
      Arduino가 실시간 balance loop를 담당합니다. ROS 쪽 명령은 직접적인 모터 권한이 아니라 상위 수준 입력으로 다룹니다.
    </td>
    <td valign="top">
      <strong>하드웨어 배선</strong><br>
      36V power, ODrive, Arduino, BNO055, FrSky receiver, onboard PC, Gemini 330 camera, auxiliary electronics, motors를 하나의 그림으로 정리했습니다.
    </td>
  </tr>
</table>

주요 실제 하드웨어는 Arduino Mega 2560, BNO055 IMU, ODrive 3.6, FrSky Taranis Q X7 + X8R, Orbbec Gemini 330, dual hall-sensor BLDC hub motors, 36V battery, onboard mini PC, DC-DC converters, auxiliary Arduino, relay module입니다.

하드웨어 설명은 [docs/ko/hardware.md](docs/ko/hardware.md)에, 제작 과정과 연구 배경은 [docs/ko/development-process.md](docs/ko/development-process.md)에 정리했습니다.

## 저장소 구조

| 경로 | 목적 |
| --- | --- |
| `firmware/` | Arduino firmware, 실제 controller, tester sketches |
| `ros_ws/` | 시뮬레이션, 내비게이션, SLAM workflow, tuning code를 위한 main ROS workspace |
| `docs/` | hardware, development process, troubleshooting, results 중심의 포트폴리오 문서 |
| `media/` | GitHub에서 보기 쉬운 GIF, 사진, diagram |
| `archive/` | 처음 읽을 위치는 아니지만, 오래된 실험과 복구된 맥락을 보존한 공간 |

## 제목에 Arduino와 ROS가 함께 들어간 이유

이 프로젝트는 실제로 Arduino와 ROS를 모두 사용했기 때문에 제목에 `Arduino-ROS`를 남겼습니다. Arduino는 실제 로봇 제어와 일부 ROS publishing test를 담당했고, ROS/Gazebo는 simulation, visualization, SLAM, navigation workflow, integration experiment를 담당했습니다.

| Sketch | ROS 역할 | Published data |
| --- | --- | --- |
| [physical_balance_controller_ros.ino](firmware/physical_balance_controller_ros/physical_balance_controller_ros.ino) | ROS-enabled physical controller variant | `/imu`, `/odom` |
| [rc_to_ros_cmd_vel_bridge.ino](firmware/testers/rc_to_ros_cmd_vel_bridge.ino) | RC receiver to ROS command bridge | `cmd_vel` |
| [experimental_balance_controller_imu_ros.ino](archive/arduino_firmware/experimental_balance_controller_imu_ros.ino) | Archived IMU publisher experiment | `/imu` |
| [legacy_balance_controller.ino](archive/arduino_firmware/legacy_balance_controller.ino) | Archived ROS publisher trace | `/imu`, `/odom` publisher logic |

## ROS Workspace

이 저장소는 custom ROS package를 보존하지만, 모든 third-party dependency snapshot을 vendoring하지는 않습니다. 과거 workflow를 재현하려면 `rtabmap_ros`, `OrbbecSDK_ROS1`, `depthimage_to_laserscan`, `rosserial`, TurtleBot3/Realsense simulation assets 같은 외부 package를 `ros_ws/src/third_party/` 아래에 배치해야 합니다.

```bash
cd ros_ws
catkin_make
source devel/setup.bash
roslaunch robot_bringup robot_remote_lidar.launch
```

Navigation-oriented simulation workflow:

```bash
roslaunch balance_robot_workflows robot_navigation_lidar.launch
```

Workspace 사용법은 [ros_ws/README.md](ros_ws/README.md)를, controller package 구조는 [balance_robot_control/README.md](ros_ws/src/balance_robot_control/README.md)를 보면 됩니다.

## 데모 미디어

- [Physical hallway balancing GIF](media/hero/physical_balance_hallway.gif)
- [Physical obstacle-course balancing GIF](media/demos/physical_balance_obstacle_course.gif)
- [Robot-focused hallway still](media/demos/hallway_robot_only.jpg)
- [Open-front hardware photo](media/hardware/robot_open_front.png)

원본 전체 MP4는 저장소에 넣지 않았습니다. 대신 GitHub에서 가볍게 볼 수 있도록 GIF와 cropped image 중심으로 정리했습니다.

## 다음으로 읽기

1. [Physical controller](firmware/physical_balance_controller/README.md): 실제 balancing robot의 main Arduino firmware.
2. [Control algorithm](firmware/physical_balance_controller/control_algorithm.md): RC input, IMU feedback, wheel speed, safety, ODrive current control이 어떻게 연결되는지 설명합니다.
3. [Hardware](docs/ko/hardware.md): 실제 부품, wiring, power flow, layout interpretation.
4. [Development process](docs/ko/development-process.md): build timeline, subsystem bring-up, research decisions.
5. [Troubleshooting summary](docs/ko/troubleshooting.md): filtering, safety gating, ODrive isolation, staged tuning이 왜 필요했는지 설명합니다.
6. [Results and limits](docs/ko/results-and-limitations.md): 측정된 결과, code-defined settings, remaining limits.

## 한계

- 실제 balancing robot에서 end-to-end autonomous ROS navigation은 future work입니다.
- 일부 오래된 코드는 프로젝트가 어떻게 발전했는지 설명하는 데 도움이 되어 `archive/`에 남겨두었습니다.
- Third-party ROS dependencies는 이 저장소에 vendoring하지 않았습니다.
- Raw process files, chat exports, full-length videos는 요약하거나 가벼운 public asset으로 대체했습니다.
