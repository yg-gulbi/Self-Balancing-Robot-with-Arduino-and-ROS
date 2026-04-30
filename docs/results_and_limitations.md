# Final Results And Claim Boundary

This page is the repository's scope boundary. It separates what was completed, what was implemented in simulation, what remains partial integration evidence, and what is not claimed.

## Claim Boundary

| Status | Area | Public claim | Main evidence |
| --- | --- | --- | --- |
| Completed | Physical self-balancing and RC driving | The real two-wheeled robot balanced and drove under Arduino control. | [`physical_balance_controller.ino`](../firmware/physical_balance_controller/physical_balance_controller.ino), [`physical_balance_hallway.gif`](../media/hero/physical_balance_hallway.gif), [`physical_balance_obstacle_course.gif`](../media/demos/physical_balance_obstacle_course.gif) |
| Completed | Arduino-to-ROS bridge evidence | Arduino sketches published robot or command data into ROS through `rosserial`-style workflows. | [`physical_balance_controller_ros.ino`](../firmware/physical_balance_controller_ros/physical_balance_controller_ros.ino), [`rc_to_ros_cmd_vel_bridge.ino`](../firmware/testers/rc_to_ros_cmd_vel_bridge.ino), [`archive/arduino_firmware`](../archive/arduino_firmware/README.md) |
| Completed | Hardware and subsystem bring-up | ODrive, BNO055, FrSky receiver, motor feedback, and power/control packaging were brought up as part of the physical robot. | [`hardware.md`](hardware.md), [`experiments.md`](experiments.md), [`development_process.md`](development_process.md) |
| Implemented in simulation | ROS/Gazebo balancing workflow | The balancing robot was modeled and controlled in Gazebo with ROS control nodes. | [`ros_ws/README.md`](../ros_ws/README.md), [`balance_robot_control`](../ros_ws/src/balance_robot_control/README.md), [`balance_robot_workflows`](../ros_ws/src/balance_robot_workflows/README.md) |
| Implemented in simulation | Navigation command pipeline | Navigation output was routed through `/before_vel` before reaching final `/cmd_vel`. | [`ros_ws/README.md`](../ros_ws/README.md), [`navigation`](../ros_ws/src/navigation/README.md) |
| Implemented in simulation | SLAM/navigation launch workflow | Depth-camera and SLAM-oriented launch composition exists as a simulation/integration workflow. | [`robot_slam_depth.launch`](../ros_ws/src/balance_robot_workflows/launch/robot_slam_depth.launch), [`ros_ws/README.md`](../ros_ws/README.md) |
| Partial integration | Real-world ROS camera/SLAM/navigation | Camera, TF, RViz, SLAM, and navigation bring-up traces exist, but full physical autonomous navigation is not claimed. | [`archive/ros_experiments/real_world_integration`](../archive/ros_experiments/real_world_integration/README.md) |
| Not claimed | End-to-end autonomous navigation on the physical robot | The repository does not claim a verified physical robot that autonomously maps, localizes, plans, and drives to goals while balancing. | This page and [`README.md`](../README.md) |
| Not claimed | Fully reproducible hardware rebuild | The cleaned repository documents the project and preserves evidence, but it is not a complete manufacturing or wiring reproduction package. | [`hardware.md`](hardware.md), [`archive/README.md`](../archive/README.md) |

## Why This Boundary Matters

The strongest result is the physical balancing robot and the control work around it. The ROS side is valuable because it shows simulation, navigation adaptation, tuning workflows, and integration experiments, but it should not be read as proof of completed autonomous navigation on the real robot.

This distinction protects the project from overclaiming while still showing the full-stack robotics process:

- low-level embedded balancing control
- noisy sensor and RC signal handling
- actuator and feedback bring-up
- ROS/Gazebo simulation and workflow composition
- navigation command adaptation for an unstable balancing platform
- partial real-world ROS integration around camera, TF, and robot-state publishing

## Current Gaps

- Full physical autonomous ROS navigation is not verified in this repository.
- Third-party ROS dependencies are documented but not vendored.
- Full-length original videos are replaced by lightweight public-safe GIFs and stills.
- Historical workspaces are preserved under `archive/`, not presented as the main runnable stack.
