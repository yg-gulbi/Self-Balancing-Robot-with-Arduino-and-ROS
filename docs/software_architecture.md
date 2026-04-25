# Software Architecture

## Overview

The project evolved into three related software tracks:

1. Physical robot firmware on Arduino
2. ROS/Gazebo simulation for balancing, SLAM, and navigation
3. Real-world ROS integration experiments for camera/SLAM/navigation bring-up

## Architecture Summary

```mermaid
flowchart LR
    RC[RC Receiver] --> Arduino[Arduino Firmware]
    IMU[BNO055 IMU] --> Arduino
    Arduino --> ODrive[ODrive Current Control]
    ODrive --> Motors[Wheel Motors]

    Teleop[Teleop or move_base] --> BeforeVel[/before_vel/]
    BeforeVel --> Balance[robot_controll]
    Balance --> CmdVel[/cmd_vel/]
    CmdVel --> Gazebo[Gazebo Robot]

    Camera[Camera / Sensor Stack] --> SLAM[SLAM / TF / Navigation Launches]
```

## Physical Robot Path

Representative code:

- [`Chimp4.ino`](../firmware/physical_balance_controller/Chimp4.ino)
- [`chimp_32.ino`](../archive/legacy_firmware/chimp_32.ino)
- [`sketch_jan20a.ino`](../archive/legacy_firmware/sketch_jan20a.ino)

Behavior:

- Arduino reads RC PWM inputs.
- BNO055 provides body angle and gyro information.
- ODrive provides motor state and accepts current commands.
- The balancing and driving loop runs entirely on Arduino for the physical robot.

## Simulation Path

Representative packages:

- [`balance_robot_bringup`](../ros_ws/src/balance_robot_bringup)
- [`balance_robot_gazebo`](../ros_ws/src/balance_robot_gazebo)
- [`robot_controll`](../ros_ws/src/robot_controll)
- [`navigation`](../ros_ws/src/navigation)

Core idea:

`move_base` or teleop does not directly drive the balancing robot. Instead, it publishes a higher-level command to `/before_vel`, and the balancing controller converts that into the final `/cmd_vel` sent to the simulated robot.

This separation is one of the most important architectural choices in the project.

## Real-World ROS Integration Experiments

Representative archive files:

- [`robot_slam.launch`](../archive/legacy_code/real_world_integration/robot_slam.launch)
- [`robot_navigation_lidar.launch`](../archive/legacy_code/real_world_integration/robot_navigation_lidar.launch)
- [`move_base.launch`](../archive/legacy_code/real_world_integration/move_base.launch)

These files show that camera/SLAM/navigation integration work was performed, but this repository does not claim verified end-to-end autonomous navigation on the physical robot.
