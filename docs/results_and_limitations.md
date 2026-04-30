# Results And Limits

This page separates the finished results, the simulation-side results, the real-world integration experiments, and the parts that should be treated as future work.

## Project Result Snapshot

| Area | Result | Notes |
| --- | --- | --- |
| Physical balance duration | About 1 hour | Real robot standing balance under Arduino control |
| Physical driving | 10 m hallway driving | Real robot driving while balancing |
| Obstacle driving | Completed on a simple indoor obstacle course | Shown through the obstacle-course demo media |
| ROS/Gazebo navigation | Robot motion checked through the navigation command path | High-level commands were routed through `/before_vel` before final `/cmd_vel` |
| ROS/Gazebo SLAM | Map generation checked | Depth SLAM workflow was used for map-generation testing |
| Arduino-to-ROS bridge | `/imu`, `/odom`, and `cmd_vel` checked with `rostopic echo` | rosserial-style bridge sketches are kept in firmware and archive folders |

## Code-Defined Settings

| Area | Value | Why it matters |
| --- | --- | --- |
| Physical tilt cutoff | `30 deg` | Set just above the protection bracket contact angle of about `28 deg`, so the controller cuts motor output before a hard fall can damage the robot |
| ODrive current clamp | `+/-8 A` | Prevents extreme current commands during recovery or unstable states |
| Firmware target speed limit | `1.5 m/s` | This is a target limit in code, not a measured top speed |
| RC neutral deadband | `+/-50 us` | Prevents small PWM drift around neutral from becoming motion |
| RC neutral offsets | throttle `1488 us`, steering `1492 us` | The controller uses calibrated centers instead of assuming exact `1500 us` |
| RC filter alpha | throttle/steering `0.4`, engage `0.02` | Throttle and steering stay responsive while engage is filtered more strongly |
| Main balance gains | `K_theta=24`, `K_theta_dot=1.7`, `Ki=0` | LQR-style body angle and angular velocity feedback |
| Speed-loop gains | `Kp=1.2`, `Ki=0.1`, `Kd=7` | Converts target motion into a balance-compatible speed correction |
| ROS publish interval | `50 ms`, about `20 Hz` | ROS-enabled Arduino variant publishes `/imu` and `/odom` at a fixed interval |
| ROS LiDAR controller gains | `Kp_angle=6.0`, `Kd_angle=-0.75`, `Kp_speed=0.3` | Main LiDAR simulation controller setting |
| ROS depth controller gains | `Kp_angle=8.0`, `Kd_angle=-1.45`, `Kp_speed=0.4` | Depth-camera simulation controller setting |
| Bayesian tuning search | `Kp_angle 2.0-8.0`, `Kd_angle -2.0 to -0.05`, `Kp_speed 0.1-0.5` | PID tuning was explored with a defined search space |

## Summary

| Status | Area | What I can say honestly | Where to look |
| --- | --- | --- | --- |
| Finished | Physical self-balancing and RC driving | The real two-wheeled robot balanced for about 1 hour and drove through a 10 m hallway and obstacle course. | [`physical_balance_controller.ino`](../firmware/physical_balance_controller/physical_balance_controller.ino), [`physical_balance_hallway.gif`](../media/hero/physical_balance_hallway.gif), [`physical_balance_obstacle_course.gif`](../media/demos/physical_balance_obstacle_course.gif) |
| Finished | Arduino-to-ROS bridge tests | Arduino sketches published robot or command data into ROS, and `/imu`, `/odom`, and `cmd_vel` were checked with `rostopic echo`. | [`physical_balance_controller_ros.ino`](../firmware/physical_balance_controller_ros/physical_balance_controller_ros.ino), [`rc_to_ros_cmd_vel_bridge.ino`](../firmware/testers/rc_to_ros_cmd_vel_bridge.ino), [`archive/arduino_firmware`](../archive/arduino_firmware/README.md) |
| Finished | Hardware and subsystem bring-up | ODrive, BNO055, FrSky receiver, motor feedback, and the main power/control packaging were all brought up on the real robot. | [`hardware.md`](hardware.md), [`experiments.md`](experiments.md), [`development_process.md`](development_process.md) |
| Built in simulation | ROS/Gazebo balancing workflow | The balancing robot was modeled and controlled in Gazebo with ROS control nodes. | [`ros_ws/README.md`](../ros_ws/README.md), [`balance_robot_control`](../ros_ws/src/balance_robot_control/README.md), [`balance_robot_workflows`](../ros_ws/src/balance_robot_workflows/README.md) |
| Built in simulation | Navigation command pipeline | Navigation output was routed through `/before_vel`, and robot motion through that path was checked. | [`ros_ws/README.md`](../ros_ws/README.md), [`navigation`](../ros_ws/src/navigation/README.md) |
| Built in simulation | SLAM and navigation workflow | Depth-camera SLAM workflow was used to check map generation. | [`robot_slam_depth.launch`](../ros_ws/src/balance_robot_workflows/launch/robot_slam_depth.launch), [`ros_ws/README.md`](../ros_ws/README.md) |
| Integration experiment | Real-world ROS camera/SLAM/navigation | Camera, TF, RViz, SLAM, and navigation bring-up traces exist for the physical robot side. | [`archive/ros_experiments/real_world_integration`](../archive/ros_experiments/real_world_integration/README.md) |
| Future work | End-to-end autonomous navigation on the physical robot | The next step would be a real balancing robot that maps, localizes, plans, and drives to goals fully on its own. | This page and [`README.md`](../README.md) |
| Future work | Fully reproducible hardware rebuild package | The repository explains the robot well, but it is not a complete manufacturing or wiring reproduction package. | [`hardware.md`](hardware.md), [`archive/README.md`](../archive/README.md) |

## How I Read The Result

The strongest part of this project is the physical balancing robot and the control work around it. The ROS side also matters because it shows simulation, navigation adaptation, tuning workflows, map-generation testing, and Arduino-to-ROS topic bridging.

The repository shows the full robotics process:

- low-level embedded balancing control
- noisy sensor and RC signal handling
- actuator and feedback bring-up
- ROS/Gazebo simulation and workflow composition
- navigation command adaptation for an unstable balancing platform
- real-world ROS integration experiments around camera, TF, and robot-state publishing

## Remaining Limits

- Full physical autonomous ROS navigation remains future work.
- Third-party ROS dependencies are documented but not vendored.
- Full-length original videos are replaced by lighter GIFs and stills.
- Older workspaces are kept under `archive/`, not presented as the main runnable stack.
