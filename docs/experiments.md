# Experiments

## Experiment Summary

| Experiment | Environment | Result | Representative evidence | Notes |
| --- | --- | --- | --- | --- |
| Physical balancing | Real robot | Completed | [`physical_balance_controller.ino`](../firmware/physical_balance_controller/physical_balance_controller.ino), [`physical_balance_hallway.gif`](../media/hero/physical_balance_hallway.gif) | Main physical balancing controller |
| Balancing while driving | Real robot | Completed | [`physical_balance_controller.ino`](../firmware/physical_balance_controller/physical_balance_controller.ino), [`physical_balance_obstacle_course.gif`](../media/demos/physical_balance_obstacle_course.gif) | RC-based driving while balancing |
| FrSky X8R receiver testing | Real robot | Completed | [`receiver_pwm_test.ino`](../firmware/testers/receiver_pwm_test/receiver_pwm_test.ino), [`rc_to_ros_cmd_vel_bridge.ino`](../firmware/testers/rc_to_ros_cmd_vel_bridge.ino), [`experimental_balance_controller_imu_ros.ino`](../archive/arduino_firmware/experimental_balance_controller_imu_ros.ino) | PWM input handling and command mapping from the Taranis Q X7 / X8R radio pair |
| FrSky X8R receiver interference troubleshooting | Real robot | Investigated | [`research_and_design_decisions.md`](research_and_design_decisions.md) | Wire length, grounding, shielding, and filtering were reviewed as signal-integrity risks |
| IMU testing | Real robot | Completed | [`experimental_balance_controller_imu_ros.ino`](../archive/arduino_firmware/experimental_balance_controller_imu_ros.ino) | ROS publishing and IMU handling experiments |
| ODrive bring-up and motor control testing | Real robot | Completed | [`motor_current_test.ino`](../firmware/testers/motor_current_test/motor_current_test.ino), [`odrive_receiver_test.ino`](../firmware/testers/odrive_receiver_test/odrive_receiver_test.ino), [`physical_balance_controller.ino`](../firmware/physical_balance_controller/physical_balance_controller.ino), [`legacy_balance_controller.ino`](../archive/arduino_firmware/legacy_balance_controller.ino), [`development_process.md`](development_process.md) | ODrive tooling, voltage checks, motor/hall-sensor setup, current control, and speed feedback loops |
| Hall-sensor and motor synchronization | Real robot | Completed | [`hall_sensor_test.ino`](../firmware/testers/hall_sensor_test/hall_sensor_test.ino), [`physical_balance_controller.ino`](../firmware/physical_balance_controller/physical_balance_controller.ino), [`development_process.md`](development_process.md) | Hall-sensor speed feedback was used with IMU feedback during balancing bring-up |
| ROS balance simulation | Simulation | Completed | [`balance_robot_control`](../ros_ws/src/balance_robot_control), [`balance_robot_gazebo`](../ros_ws/src/balance_robot_gazebo) | Gazebo-based balancing stack |
| Simulation SLAM/navigation launch workflow | Simulation | Implemented | [`balance_robot_workflows`](../ros_ws/src/balance_robot_workflows), [`research_and_design_decisions.md`](research_and_design_decisions.md) | Launch composition and workflow experiments informed by ORB-SLAM2 and RTAB-Map research |
| Simulation navigation | Simulation | Completed | [`navigation`](../ros_ws/src/navigation), [`balance_robot_workflows`](../ros_ws/src/balance_robot_workflows) | `move_base` integrated with the `/before_vel` design |
| Orbbec Gemini 330 evaluation | Research / integration planning | Investigated | [`research_and_design_decisions.md`](research_and_design_decisions.md), [`hardware.md`](hardware.md) | Gemini 330 integration informed the sensor-head story, but physical autonomous depth navigation is not claimed |
| PID tuning and auto-tuning | Simulation | Completed | [`balance_robot_control`](../ros_ws/src/balance_robot_control), [`archive/ros_experiments/pid_experiments`](../archive/ros_experiments/pid_experiments) | Manual and automated tuning experiments |
| RC-to-ROS bridge | Integration test | Completed | [`rc_to_ros_cmd_vel_bridge.ino`](../firmware/testers/rc_to_ros_cmd_vel_bridge.ino) | Used as a tester, not the final robot controller |
| Real-world ROS SLAM/navigation setup | Integration test | Partial | [`archive/ros_experiments/real_world_integration`](../archive/ros_experiments/real_world_integration) | Setup exists, full autonomous physical navigation not claimed |

## Reading Guide

- Use `firmware/` when you want the physical robot controller and bridge code.
- Use `ros_ws/src/` when you want the cleaned ROS simulation/control stack.
- Use `archive/` when you want the evolution history, older versions, and raw evidence.
