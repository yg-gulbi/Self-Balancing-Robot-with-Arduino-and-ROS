# Arduino Firmware Archive

This folder preserves older or alternate Arduino sketches that support the physical robot and Arduino-to-ROS history.

- [`legacy_balance_controller.ino`](legacy_balance_controller.ino): earlier balancing controller variant with preserved `rosserial` traces for `/imu` and `/odom` publisher setup and helper functions; some publish calls are commented in this archived sketch
- [`experimental_balance_controller_imu_ros.ino`](experimental_balance_controller_imu_ros.ino): experimental firmware with `rosserial` IMU publishing and balance-controller work

Use this archive for historical evidence. The default physical controller is [`../../firmware/physical_balance_controller`](../../firmware/physical_balance_controller).
