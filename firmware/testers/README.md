# Bridges And Testers

This folder contains Arduino sketches that supported testing, integration, or bridging workflows.

The clearest Arduino-to-ROS example here is `rc_to_ros_cmd_vel_bridge.ino`, which uses `rosserial` (`ros.h`) to publish a `geometry_msgs/Twist` command from RC receiver PWM input.

- [`rc_to_ros_cmd_vel_bridge.ino`](rc_to_ros_cmd_vel_bridge.ino): RC PWM input to ROS `cmd_vel` bridge used as a tester for ROS-side experiments
- [`hall_sensor_test`](hall_sensor_test/hall_sensor_test.ino): three-line BLDC hall sensor state and illegal-state counter, based on the ChIMP hall test approach
- [`motor_current_test`](motor_current_test/motor_current_test.ino): direct ODrive current command bench test for one or both motors, based on the ChIMP motor test command workflow
- [`odrive_receiver_test`](odrive_receiver_test/odrive_receiver_test.ino): ODrive current-control test driven by RC receiver PWM inputs, adapted from HoverBot's ODrive test workflow
- [`receiver_pwm_test`](receiver_pwm_test/receiver_pwm_test.ino): FrSky receiver PWM pulse-width inspection sketch, based on HoverBot's receiver test workflow
- [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md): attribution and license notes for referenced public test code

These files are useful evidence for subsystem testing, but they are not the main physical balancing controller.
