# Archive

This folder keeps historical project material that is useful as evidence, but is not the default starting point for understanding or running the robot.

First-time reviewers should start with [`../README.md`](../README.md), [`../firmware/physical_balance_controller`](../firmware/physical_balance_controller/README.md), and [`../ros_ws`](../ros_ws/README.md). Use this archive only when you want the development history behind those cleaned entrypoints.

## Folders

| Folder | What it contains | When to read it |
| --- | --- | --- |
| [`ros_experiments`](ros_experiments/README.md) | Earlier ROS PID controllers, real-world launch files, TF helpers, camera/SLAM/navigation bring-up traces, and workspace history | When reviewing how the ROS side evolved |
| [`arduino_firmware`](arduino_firmware/README.md) | Older Arduino balance-controller variants and `rosserial` publisher experiments | When checking Arduino-to-ROS history |
| [`raw_documents`](raw_documents/README.md) | Preserved source material, including the project trial-and-error PDF and receiver-interference notes | Only when raw recovery evidence is needed |

For the cleaned main code paths, start with [`../firmware`](../firmware), [`../ros_ws`](../ros_ws), and [`../README.md`](../README.md).
