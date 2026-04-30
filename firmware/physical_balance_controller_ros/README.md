# Physical Balance Controller + ROS

This sketch is a ROS-enabled variant of the main physical balancing controller.

- Main file: [`physical_balance_controller_ros.ino`](physical_balance_controller_ros.ino)
- Base controller: [`../physical_balance_controller/physical_balance_controller.ino`](../physical_balance_controller/physical_balance_controller.ino)
- Archived ROS source: [`../../archive/arduino_firmware/legacy_balance_controller.ino`](../../archive/arduino_firmware/legacy_balance_controller.ino)

## Purpose

The original physical controller keeps the real-time balance loop fully on Arduino for safety and timing. This variant keeps that architecture, then adds the archived `rosserial` publishing path so the physical robot can expose state to ROS:

| Topic | Message | Source |
| --- | --- | --- |
| `/imu` | `sensor_msgs/Imu` | BNO055 orientation, gyro, and linear acceleration |
| `/odom` | `nav_msgs/Odometry` | Integrated wheel-speed odometry from ODrive feedback |

## Control And Communication Structure

The important design choice is that ROS does not close the balance loop. Arduino still performs:

- RC PWM decoding for throttle, steering, and engage
- IMU tilt feedback and tilt cutoff
- wheel-speed feedback from ODrive
- balance correction, speed correction, steering correction, and current limiting
- ODrive closed-loop state switching and current commands

ROS receives state for visualization, logging, mapping, or higher-level integration. This avoids depending on USB/ROS timing for the unstable balancing loop.

## Usage

Upload this sketch instead of the non-ROS physical controller when you want ROS state topics:

```bash
roscore
rosrun rosserial_python serial_node.py _port:=/dev/ttyACM0 _baud:=115200
rostopic echo /imu
rostopic echo /odom
```

Adjust `/dev/ttyACM0` to the Arduino port on your machine.

## Notes

- `Serial` is owned by `rosserial` in this variant. Avoid normal `Serial.print()` debugging while connected to ROS because it can corrupt the rosserial frame stream.
- The serial gain-tuning command processor is left in the code but disabled by default with `kSerialGainTuningEnabled = false`.
- `/odom` is wheel-speed integration, so it will drift over time. It is useful as a ROS integration signal, not as a globally accurate pose estimate.
