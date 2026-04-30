# Software Architecture

This page is a compact map of the project structure. Detailed control logic lives in [`control_algorithm.md`](../firmware/physical_balance_controller/control_algorithm.md), ROS launch usage lives in [`ros_ws/README.md`](../ros_ws/README.md), and final claims are bounded in [`results_and_limitations.md`](results_and_limitations.md).

## Visual Architecture

<p align="center">
  <img src="../media/diagrams/signal_control_diagram.png" alt="Signal / Control Diagram" width="900">
</p>

## Three Paths

| Path | Role | Main entrypoint |
| --- | --- | --- |
| Physical firmware path | Arduino owns the real-time balance loop, reads RC/IMU feedback, and commands ODrive current. | [`firmware/physical_balance_controller`](../firmware/physical_balance_controller/README.md) |
| ROS simulation path | Gazebo, balancing controllers, navigation, SLAM workflow launches, and PID tuning run in the curated ROS workspace. | [`ros_ws/README.md`](../ros_ws/README.md) |
| Archived real-world integration path | Camera, TF, RViz, SLAM, navigation, and Arduino-to-ROS traces are preserved as integration evidence. | [`archive/README.md`](../archive/README.md) |

## Control Authority

The important architecture decision is that high-level commands do not bypass the balancing layer.

```text
teleop or move_base
  -> /before_vel
  -> balance-aware controller
  -> /cmd_vel
  -> simulated robot
```

The physical robot follows the same philosophy at a lower level: Arduino remains close to the unstable hardware, and ROS-side work is treated as higher-level intent, visualization, or integration evidence.

## Scope Note

Real physical balancing and RC driving are completed results. ROS/Gazebo navigation and SLAM workflows are simulation-side or integration-side results. Verified end-to-end autonomous navigation on the physical balancing robot is not claimed here.
