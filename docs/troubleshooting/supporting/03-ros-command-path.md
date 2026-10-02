# ROS navigation command path and balance authority

[한국어](../../ko/troubleshooting/supporting/03-ros-command-path.md) | [Troubleshooting index](../../troubleshooting.md)

**Priority:** Supporting · Reconstructed from existing documentation and code

## 1. Problem definition

Navigation requests motion, while a two-wheeled balancing base must continuously stabilize its body. Sending navigation velocity directly to the simulated base bypasses the layer that reconciles those requirements. This entry describes an architecture problem, not a separately measured hardware failure.

## 2. Candidate solutions

Separate desired motion from the final base command. Route teleoperation and navigation into an intent topic; have the balancing controller combine intent with IMU/odometry feedback and publish the final command. Preserve that control boundary when discussing physical integration.

## 3. Execution

The ROS simulation package consumes `/before_vel`, `/imu`, and `/odom` and publishes `/cmd_vel`. The documented navigation route is `move_base -> /before_vel -> balance_robot_control -> /cmd_vel`. Controller and launch files are the evidence for routing. On hardware, the Arduino balance/safety loop sends ODrive current commands; a matching design principle does not establish completed autonomous physical navigation.

## 4. Reinterpreting the experience

Topic separation makes command responsibility explicit. It is useful only if launch remapping and active publishers actually preserve the path; topic names alone do not enforce the boundary.

## 5. Summary and verification

Outcome: the intent/controller/output route is documented and implemented for simulation; physical autonomous navigation remains integration work. Review the package, navigation launch files, and Sim2Real limits. In a running simulation, inspect `rostopic info /before_vel` and `rostopic info /cmd_vel`, then compare intent, pitch, and output during a commanded move and stop.

### Evidence

- [Balance control package](../../../ros_ws/src/balance_robot_control/README.md)
- [Navigation package](../../../ros_ws/src/navigation)
- [Sim2Real boundaries](../../../docs/sim2real.md)
