# 결과와 한계

[English](../results-and-limitations.md) | 한국어

이 문서는 finished results, simulation-side results, real-world integration experiments, future work를 분리해서 설명합니다.

## Project Result Snapshot

| Area | Result | Notes |
| --- | --- | --- |
| Physical balance duration | 약 1시간 | Arduino control 아래 실제 로봇 standing balance |
| Physical driving | 10 m hallway driving | 균형을 유지하면서 실제 로봇 주행 |
| Obstacle driving | 간단한 실내 장애물 코스 완료 | Obstacle-course demo media로 확인 가능 |
| ROS/Gazebo navigation | Navigation command path를 통한 robot motion 확인 | High-level command가 final `/cmd_vel` 전에 `/before_vel`을 거치도록 구성 |
| ROS/Gazebo SLAM | Map generation 확인 | Depth SLAM workflow로 map-generation test 수행 |
| Arduino-to-ROS bridge | `/imu`, `/odom`, `cmd_vel`을 `rostopic echo`로 확인 | rosserial-style bridge sketch는 firmware와 archive folder에 보존 |

## Evidence Map

| Area | Environment | Evidence | How to read it |
| --- | --- | --- | --- |
| Physical balancing and RC driving | Real robot | [`physical_balance_controller.ino`](../../firmware/physical_balance_controller/physical_balance_controller.ino), hallway/obstacle-course demo media | Completed real-world result |
| RC receiver and command handling | Real robot | [`receiver_pwm_test.ino`](../../firmware/testers/receiver_pwm_test/receiver_pwm_test.ino), [`rc_to_ros_cmd_vel_bridge.ino`](../../firmware/testers/rc_to_ros_cmd_vel_bridge.ino) | Completed subsystem tests and bridge workflow |
| ODrive, hall feedback, and motor control | Real robot | [`motor_current_test.ino`](../../firmware/testers/motor_current_test/motor_current_test.ino), [`odrive_receiver_test.ino`](../../firmware/testers/odrive_receiver_test/odrive_receiver_test.ino), [`development-process.md`](development-process.md) | Physical controller 뒤의 completed bring-up path |
| ROS balance simulation and tuning | Simulation | [`balance_robot_control`](../../ros_ws/src/balance_robot_control), [`balance_robot_gazebo`](../../ros_ws/src/balance_robot_gazebo), archived PID experiments | Completed software-side workflow |
| SLAM and navigation workflow | Simulation / integration | [`balance_robot_workflows`](../../ros_ws/src/balance_robot_workflows), [`navigation`](../../ros_ws/src/navigation), [`archive/ros_experiments/real_world_integration`](../../archive/ros_experiments/real_world_integration/README.md) | Simulation result와 partial physical integration context |

## Code-Defined Settings

| Area | Value | Why it matters |
| --- | --- | --- |
| Physical tilt cutoff | `30 deg` | 보호 브래킷이 약 `28 deg`에서 바닥에 닿기 때문에, hard fall로 인한 손상 전에 motor output을 차단 |
| ODrive current clamp | `+/-8 A` | Recovery 또는 unstable state에서 과도한 current command를 방지 |
| Firmware target speed limit | `1.5 m/s` | 측정된 최고 속도가 아니라 code 안의 target limit |
| RC neutral deadband | `+/-50 us` | Neutral 주변의 작은 PWM drift가 motion으로 바뀌는 것을 방지 |
| RC neutral offsets | throttle `1488 us`, steering `1492 us` | 정확히 `1500 us`를 가정하지 않고 calibrated center 사용 |
| RC filter alpha | throttle/steering `0.4`, engage `0.02` | Throttle/steering은 responsive하게, engage는 더 강하게 filtering |
| Main balance gains | `K_theta=24`, `K_theta_dot=1.7`, `Ki=0` | Body angle과 angular velocity 기반 LQR-style feedback |
| Speed-loop gains | `Kp=1.2`, `Ki=0.1`, `Kd=7` | Target motion을 balance-compatible speed correction으로 변환 |
| ROS publish interval | `50 ms`, about `20 Hz` | ROS-enabled Arduino variant가 `/imu`, `/odom`을 고정 주기로 publish |
| ROS LiDAR controller gains | `Kp_angle=6.0`, `Kd_angle=-0.75`, `Kp_speed=0.3` | Main LiDAR simulation controller setting |
| ROS depth controller gains | `Kp_angle=8.0`, `Kd_angle=-1.45`, `Kp_speed=0.4` | Depth-camera simulation controller setting |
| Bayesian tuning search | `Kp_angle 2.0-8.0`, `Kd_angle -2.0 to -0.05`, `Kp_speed 0.1-0.5` | PID tuning을 정의된 search space로 탐색 |

## Summary

| Status | Area | What I can say honestly | Where to look |
| --- | --- | --- | --- |
| Finished | Physical self-balancing and RC driving | 실제 2륜 로봇이 약 1시간 균형을 유지했고, 10 m hallway와 obstacle course를 주행했습니다. | [`physical_balance_controller.ino`](../../firmware/physical_balance_controller/physical_balance_controller.ino), [`physical_balance_hallway.gif`](../../media/hero/physical_balance_hallway.gif), [`physical_balance_obstacle_course.gif`](../../media/demos/physical_balance_obstacle_course.gif) |
| Finished | Arduino-to-ROS bridge tests | Arduino sketch가 robot/command data를 ROS로 publish했고, `/imu`, `/odom`, `cmd_vel`을 `rostopic echo`로 확인했습니다. | [`physical_balance_controller_ros.ino`](../../firmware/physical_balance_controller_ros/physical_balance_controller_ros.ino), [`rc_to_ros_cmd_vel_bridge.ino`](../../firmware/testers/rc_to_ros_cmd_vel_bridge.ino), [`archive/arduino_firmware`](../../archive/arduino_firmware/README.md) |
| Finished | Hardware and subsystem bring-up | ODrive, BNO055, FrSky receiver, motor feedback, main power/control packaging을 실제 로봇에서 bring-up했습니다. | [`hardware.md`](hardware.md), [`development-process.md`](development-process.md) |
| Built in simulation | ROS/Gazebo balancing workflow | Balancing robot을 Gazebo에서 model/control했습니다. | [`ros_ws/README.md`](../../ros_ws/README.md), [`balance_robot_control`](../../ros_ws/src/balance_robot_control/README.md), [`balance_robot_workflows`](../../ros_ws/src/balance_robot_workflows/README.md) |
| Built in simulation | Navigation command pipeline | Navigation output을 `/before_vel`로 routing했고, 그 path를 통한 robot motion을 확인했습니다. | [`ros_ws/README.md`](../../ros_ws/README.md), [`navigation`](../../ros_ws/src/navigation/README.md) |
| Built in simulation | SLAM and navigation workflow | Depth-camera SLAM workflow로 map generation을 확인했습니다. | [`robot_slam_depth.launch`](../../ros_ws/src/balance_robot_workflows/launch/robot_slam_depth.launch), [`ros_ws/README.md`](../../ros_ws/README.md) |
| Integration experiment | Real-world ROS camera/SLAM/navigation | Physical robot side의 camera, TF, RViz, SLAM, navigation bring-up trace가 존재합니다. | [`archive/ros_experiments/real_world_integration`](../../archive/ros_experiments/real_world_integration/README.md) |
| Future work | End-to-end autonomous navigation on the physical robot | 다음 단계는 실제 balancing robot이 map, localize, plan, goal driving까지 완전히 수행하는 것입니다. | 이 문서와 [`README.ko.md`](../../README.ko.md) |
| Future work | Fully reproducible hardware rebuild package | 저장소는 로봇을 잘 설명하지만, 완전한 manufacturing/wiring reproduction package는 아닙니다. | [`hardware.md`](hardware.md), [`archive/README.md`](../../archive/README.md) |

## How I Read The Result

이 프로젝트에서 가장 강한 부분은 실제 balancing robot과 그 주변의 control work입니다. ROS 쪽도 중요합니다. Simulation, navigation adaptation, tuning workflow, map-generation testing, Arduino-to-ROS topic bridging을 보여주기 때문입니다.

이 저장소가 보여주는 전체 robotics process는 다음과 같습니다.

- low-level embedded balancing control
- noisy sensor와 RC signal handling
- actuator와 feedback bring-up
- ROS/Gazebo simulation과 workflow composition
- unstable balancing platform을 위한 navigation command adaptation
- camera, TF, robot-state publishing 주변의 real-world ROS integration experiments

## Remaining Limits

- Full physical autonomous ROS navigation은 future work입니다.
- Third-party ROS dependencies는 documented but not vendored입니다.
- Full-length original video는 lighter GIF와 still image로 대체했습니다.
- Older workspace는 main runnable stack이 아니라 `archive/` 아래에 보존했습니다.
