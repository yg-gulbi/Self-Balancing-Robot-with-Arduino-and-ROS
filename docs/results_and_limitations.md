# What I Finished And What I Didn't

I wrote this page so the project reads clearly without sounding bigger than it really was. It separates the parts I finished, the parts I built in simulation, the parts I only got partway through on the real robot, and the parts I do not want to pretend were completed.

## Summary

| Status | Area | What I can say honestly | Where to look |
| --- | --- | --- | --- |
| Finished | Physical self-balancing and RC driving | The real two-wheeled robot balanced and drove under Arduino control. | [`physical_balance_controller.ino`](../firmware/physical_balance_controller/physical_balance_controller.ino), [`physical_balance_hallway.gif`](../media/hero/physical_balance_hallway.gif), [`physical_balance_obstacle_course.gif`](../media/demos/physical_balance_obstacle_course.gif) |
| Finished | Arduino-to-ROS bridge tests | I made Arduino sketches that published robot or command data into ROS through `rosserial`-style workflows. | [`physical_balance_controller_ros.ino`](../firmware/physical_balance_controller_ros/physical_balance_controller_ros.ino), [`rc_to_ros_cmd_vel_bridge.ino`](../firmware/testers/rc_to_ros_cmd_vel_bridge.ino), [`archive/arduino_firmware`](../archive/arduino_firmware/README.md) |
| Finished | Hardware and subsystem bring-up | ODrive, BNO055, FrSky receiver, motor feedback, and the main power/control packaging were all brought up on the real robot. | [`hardware.md`](hardware.md), [`experiments.md`](experiments.md), [`development_process.md`](development_process.md) |
| Built in simulation | ROS/Gazebo balancing workflow | The balancing robot was modeled and controlled in Gazebo with ROS control nodes. | [`ros_ws/README.md`](../ros_ws/README.md), [`balance_robot_control`](../ros_ws/src/balance_robot_control/README.md), [`balance_robot_workflows`](../ros_ws/src/balance_robot_workflows/README.md) |
| Built in simulation | Navigation command pipeline | Navigation output was routed through `/before_vel` before reaching final `/cmd_vel`. | [`ros_ws/README.md`](../ros_ws/README.md), [`navigation`](../ros_ws/src/navigation/README.md) |
| Built in simulation | SLAM and navigation launch workflow | Depth-camera and SLAM-oriented launch composition exists on the simulation and integration side. | [`robot_slam_depth.launch`](../ros_ws/src/balance_robot_workflows/launch/robot_slam_depth.launch), [`ros_ws/README.md`](../ros_ws/README.md) |
| Tried on the real robot, but not fully finished | Real-world ROS camera/SLAM/navigation | Camera, TF, RViz, SLAM, and navigation bring-up traces exist, but I am not calling this full physical autonomous navigation. | [`archive/ros_experiments/real_world_integration`](../archive/ros_experiments/real_world_integration/README.md) |
| Not finished | End-to-end autonomous navigation on the physical robot | This repository does not show a real balancing robot that maps, localizes, plans, and drives to goals fully on its own. | This page and [`README.md`](../README.md) |
| Not finished | Fully reproducible hardware rebuild package | The repository explains the robot well, but it is not a complete manufacturing or wiring reproduction package. | [`hardware.md`](hardware.md), [`archive/README.md`](../archive/README.md) |

## Why I Wrote This

The strongest part of this project is the physical balancing robot and the control work around it. The ROS side still matters because it shows simulation, navigation adaptation, tuning workflows, and integration experiments, but I do not want it to read like I fully finished autonomous navigation on the real robot when I did not.

I still wanted the repository to show the full robotics process:

- low-level embedded balancing control
- noisy sensor and RC signal handling
- actuator and feedback bring-up
- ROS/Gazebo simulation and workflow composition
- navigation command adaptation for an unstable balancing platform
- partial real-world ROS integration around camera, TF, and robot-state publishing

## What Is Still Missing

- Full physical autonomous ROS navigation is not finished in this repository.
- Third-party ROS dependencies are documented but not vendored.
- Full-length original videos are replaced by lighter GIFs and stills.
- Older workspaces are kept under `archive/`, not presented as the main runnable stack.
