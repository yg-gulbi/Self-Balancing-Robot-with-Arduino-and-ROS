# Software Architecture

This page is a short map of how the project is split up. The detailed control logic is in [`control_algorithm.md`](../firmware/physical_balance_controller/control_algorithm.md), the ROS launch usage is in [`ros_ws/README.md`](../ros_ws/README.md), and the scope page is in [`results_and_limitations.md`](results_and_limitations.md).

## Visual Architecture

<p align="center">
  <img src="../media/diagrams/signal_control_diagram.png" alt="Signal / Control Diagram" width="900">
</p>

## Main Paths

| Path | Role | Main entrypoint |
| --- | --- | --- |
| Physical firmware path | Arduino owns the real-time balance loop, reads RC/IMU feedback, and commands ODrive current. | [`firmware/physical_balance_controller`](../firmware/physical_balance_controller/README.md) |
| ROS simulation path | Gazebo, balancing controllers, navigation, SLAM workflow launches, and PID tuning run in the main ROS workspace. | [`ros_ws/README.md`](../ros_ws/README.md) |
| Archived real-world integration path | Camera, TF, RViz, SLAM, navigation, and Arduino-to-ROS traces are kept in the archive if you want to see that part of the project history. | [`archive/README.md`](../archive/README.md) |

## Control Authority

The important architecture idea is that high-level commands do not bypass the balancing layer.

```text
teleop or move_base
  -> /before_vel
  -> balance-aware controller
  -> /cmd_vel
  -> simulated robot
```

The physical robot follows the same idea at a lower level. Arduino stays close to the unstable hardware, and the ROS side is used more for high-level intent, visualization, and integration work.

## How To Read It

Real physical balancing and RC driving are finished results. ROS/Gazebo navigation and SLAM workflows are simulation-side or integration-side results. I am not presenting end-to-end autonomous navigation on the physical balancing robot as completed here.
