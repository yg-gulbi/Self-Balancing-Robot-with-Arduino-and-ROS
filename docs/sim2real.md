# Sim2Real Bridge

English | [한국어](ko/sim2real.md)

This page explains how I used simulation and the physical robot together. It is intentionally conservative: the project completed physical self-balancing and RC driving, and it completed ROS/Gazebo navigation workflows in simulation. Full autonomous ROS navigation on the physical balancing robot remains future work.

## Core Idea

The main sim-to-real idea was not to let high-level motion commands bypass the balancing layer.

```text
Simulation:
teleop or move_base
  -> /before_vel
  -> balance_robot_control
  -> /cmd_vel
  -> Gazebo robot

Physical robot:
RC or ROS-side intent
  -> Arduino balance and safety loop
  -> ODrive current commands
  -> wheel motors
```

In simulation, `/before_vel` is the motion-intent topic. The balancing controller turns that intent into the final `/cmd_vel`. On the real robot, the Arduino plays the same safety role at a lower level: throttle or steering intent can ask the robot to move, but the IMU feedback, tilt cutoff, current clamp, and wheel-speed correction still decide what reaches the motors.

## What Transferred From Simulation To Hardware

| Transfer point | Simulation side | Physical side |
| --- | --- | --- |
| Robot form factor | Gazebo model followed the same two-wheeled layout and 3D-printed design direction | The real robot used the printed chassis, upper sensor structure, and internal electronics packaging |
| Command layering | Navigation and teleop publish intent before balance control | RC and ROS bridge paths are treated as commands into a local Arduino balance loop |
| Balance-aware motion | Desired velocity is converted into a lean or correction request before final motion output | RC throttle becomes a speed target that shifts the balance point instead of directly commanding wheel speed |
| Sensor workflow | Gazebo provides `/imu`, `/odom`, scan/depth, map, and RViz feedback for workflow testing | BNO055, ODrive feedback, RC PWM, and camera integration traces represent the real sensing path |
| Safety boundary | Simulation controllers stop or limit output when pitch becomes unsafe | Firmware has tilt cutoff, engage persistence, current clamp, and inactive-state reset |

## What Stayed Simulation-Only

The ROS navigation stack reached a cleaner state in Gazebo than on the physical robot. In simulation, the repository preserves launch files for control, LiDAR navigation, depth navigation, SLAM, and PID tuning. On the real robot, the strongest completed result is still Arduino-controlled balancing and RC driving.

That split matters because a balancing robot is not a normal differential-drive base. A planner can output a velocity command, but the robot must still preserve body angle, current limits, and recovery behavior. The simulation proved the command architecture and workflow composition; the physical robot proved the low-level balancing and driving behavior.

## What Needed Real-World Tuning

| Area | Why simulation was not enough | Repository evidence |
| --- | --- | --- |
| IMU angle and gyro behavior | Real calibration, mounting offset, and noise affected the balance loop | [`physical_balance_controller.ino`](../firmware/physical_balance_controller/physical_balance_controller.ino), [`control_algorithm.md`](../firmware/physical_balance_controller/control_algorithm.md) |
| RC input reliability | Receiver PWM values drifted and needed filtering, deadband, and engage persistence | [`troubleshooting.md`](troubleshooting.md) |
| Motor and wheel feedback | Hall feedback, current response, and wheel behavior had real hardware limits | [`motor_current_test.ino`](../firmware/testers/motor_current_test/motor_current_test.ino), [`hardware.md`](hardware.md) |
| Fall safety | Physical testing needed current clamps and tilt cutoff to protect the robot | [`results-and-limitations.md`](results-and-limitations.md) |
| Depth-camera integration | Camera, TF, SLAM, and navigation traces existed, but full physical autonomy was not completed | [`real-world integration archive`](../archive/ros_experiments/real_world_integration/README.md) |

## Honest Result

The project demonstrates a practical sim-to-real bridge in two directions:

- from simulation to hardware: balance-aware command separation, navigation workflow structure, robot layout, and tuning ideas
- from hardware back to simulation: safety constraints, real controller limits, and the need to keep final actuator authority below the high-level planner

The completed physical result is balancing and RC driving. The completed ROS result is simulation-side control, SLAM/navigation workflow composition, and command-path adaptation. The unfinished bridge is full end-to-end autonomous navigation on the physical balancing robot.
