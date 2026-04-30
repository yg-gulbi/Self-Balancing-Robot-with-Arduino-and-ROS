# balance_robot_control

This package is the cleaned ROS control package for the current `ros_ws`
simulation stack.

Its core idea is simple but unusual:

1. accept a desired motion command on `/before_vel`,
2. estimate how much the robot must lean to satisfy that request,
3. combine balance correction and steering correction,
4. publish the final low-level command on `/cmd_vel`.

That makes this package the center of the balancing simulation stack.

## Package Role

- consumes `/before_vel`, `/imu`, and `/odom`
- produces `/cmd_vel`
- inserts a balancing layer between navigation and the robot base
- stores both manual tuning and automatic tuning experiments
- keeps the depth-model and LiDAR-model control variants in one package

The most important system-level consequence is that `move_base` does not command the robot directly. Instead:

`move_base -> /before_vel -> balance_robot_control -> /cmd_vel`

The name `balance_robot_control` was chosen because this package contains more
than one controller script. It includes the main balance-aware control nodes,
the depth and LiDAR variants, the PID tuning entrypoints, and helper scripts
such as Bayesian optimization and repeated-loop evaluation.

## Source Layout

| Folder | Purpose |
| --- | --- |
| `src/controllers/` | Main balance controllers used during normal LiDAR or depth simulation |
| `src/tuning_controllers/` | Controller variants that expose tuning hooks such as `dynamic_reconfigure` or `/setpoint_angle` |
| `src/tuning_tools/` | Outer-loop tuning, repeated test, and reconfigure helper scripts |
| `src/utils/` | Small support utilities such as Gazebo reset and `/before_vel` teleop |

`CMakeLists.txt` installs these Python scripts with `catkin_install_python`, so launch files can still reference the executable filename directly.

## File-by-File Guide

### Active controller files

#### [`cfg/PID.cfg`](cfg/PID.cfg)

Defines the `dynamic_reconfigure` parameters used by the tuning variants:

- `Kp_angle`
- `Kd_angle`
- `Kp_speed`
- `Kp_steering`
- `Kd_steering`

This file does not run by itself. It generates the Python config class that the tuning nodes import.

#### [`src/controllers/pid_control_before_vel_lidar.py`](src/controllers/pid_control_before_vel_lidar.py)

Main LiDAR-version balancing controller.

What it does:

- subscribes to `/before_vel`, `/imu`, and `/odom`
- converts speed error into a desired lean angle
- computes pitch-rate and roll-rate from quaternion differences
- publishes final motion on `/cmd_vel`
- stops the robot if pitch exceeds about 40 degrees

Why it matters:

- this is the main expression of the balancing architecture
- it is the default controller for LiDAR-based simulation

#### [`src/controllers/pid_control_before_vel_depth.py`](src/controllers/pid_control_before_vel_depth.py)

Depth-camera version of the same controller.

What is different from the LiDAR version:

- higher pitch gains
- steering gains are slightly different
- desired lean angle is clamped to about +/-30 degrees
- an extra pitch bias term is applied in the angle error

Interpretation:

- the controller structure stayed the same
- the depth-camera simulation needed different gains and a small posture offset compensation

#### [`src/tuning_controllers/pid_control_before_vel_lidar_pid_tuning.py`](src/tuning_controllers/pid_control_before_vel_lidar_pid_tuning.py)

LiDAR controller variant used for manual PID tuning.

What it adds:

- `dynamic_reconfigure` server
- runtime PID gain edits from `rqt_reconfigure`
- simulation reset detection
- state reinitialization when Gazebo time jumps backward

Why it exists:

- Gazebo reset behavior interfered with controller tuning
- this version was built specifically to survive repeated reset-and-retest workflows

#### [`src/tuning_controllers/pid_control_before_vel_lidar_pid_auto_tuning.py`](src/tuning_controllers/pid_control_before_vel_lidar_pid_auto_tuning.py)

Controller variant prepared for automated tuning.

What it adds beyond the manual tuning version:

- publishes `/setpoint_angle`
- remains `dynamic_reconfigure` compatible
- is designed to be scored by an outer optimization loop

Why `/setpoint_angle` matters:

- the optimizer can compare the commanded lean target with the actual pitch response
- that makes balance-tracking quality measurable instead of only visually judged

### Tuning and automation helpers

#### [`src/tuning_tools/pid_bayes_optimizer.py`](src/tuning_tools/pid_bayes_optimizer.py)

Bayesian optimization driver for LiDAR PID tuning.

What it does:

- updates PID values through `dynamic_reconfigure`
- resets Gazebo using ROS services
- runs a repeatable motion sequence: stop -> forward/turn -> stop
- subscribes to `/imu`, `/odom`, and `/setpoint_angle`
- computes a score from tracking, stopping, balance, and angular errors
- applies a heavy penalty if the robot falls

Why it matters:

- this file shows the project went beyond controller implementation into tuning automation
- it is one of the strongest pieces of evidence that repeated evaluation and optimization were attempted systematically

#### [`src/tuning_tools/pid_loop_test_runner.py`](src/tuning_tools/pid_loop_test_runner.py)

Simpler repeat-test runner for a fixed PID set.

What it does:

- reconfigures the controller
- pauses/resets/unpauses Gazebo
- commands forward motion
- repeats the same loop 100 times

Why it exists:

- this is a lighter-weight precursor to the full Bayesian optimizer
- it helped check whether a candidate PID configuration could survive repeated reset cycles

#### [`src/utils/gazebo_reset_world.py`](src/utils/gazebo_reset_world.py)

Small Gazebo utility script.

What it does:

- pause physics
- reset world
- wait briefly
- unpause physics

Why it exists:

- controller tuning and loop testing needed a repeatable reset helper

#### [`src/tuning_tools/pid_reconfigure_client.py`](src/tuning_tools/pid_reconfigure_client.py)

Minimal `dynamic_reconfigure` client example.

What it does:

- connects to the PID tuning node
- updates selected PID parameters programmatically

Why it exists:

- proves the tuning node can be controlled from code, not only from `rqt_reconfigure`

### Command input helper

#### [`src/utils/teleop_before_vel.py`](src/utils/teleop_before_vel.py)

Keyboard teleoperation publisher for `/before_vel`.

Why it matters:

- the balancing controller expects desired motion on `/before_vel`
- this file provides a manual way to drive the balance stack without navigation

## Archived Experiment Lineage

Earlier controller variants were intentionally moved to [archive/ros_experiments/pid_experiments](../../../archive/ros_experiments/pid_experiments/README.md).

Those files preserve the path from rough experiments to the cleaned package above:

- [`pid_control_cmd_vel.py`](../../../archive/ros_experiments/pid_experiments/pid_control_cmd_vel.py): earliest command path, before `/before_vel` became standard
- [`pid_control_before_vel.py`](../../../archive/ros_experiments/pid_experiments/pid_control_before_vel.py): early `/before_vel` controller without odometry-based speed feedback
- [`pid_control_before_vel_odom.py`](../../../archive/ros_experiments/pid_experiments/pid_control_before_vel_odom.py): early controller that uses `/odom` speed feedback
- [`pid_control_before_vel_odom_vel.py`](../../../archive/ros_experiments/pid_experiments/pid_control_before_vel_odom_vel.py): transition experiment mixing angle control and speed-estimation ideas
- [`pid_control_before_vel_revised.py`](../../../archive/ros_experiments/pid_experiments/pid_control_before_vel_revised.py): experiment that integrates IMU linear acceleration to estimate speed

These files are useful historically because they show the control-design progression:

1. direct command experiments,
2. `/before_vel` adoption,
3. speed-feedback experiments,
4. tuning automation.

## Known Boundaries

- These controllers operate in simulation and assume `/imu` and `/odom` already exist.
- The LiDAR and depth variants are tuned separately rather than abstracted into one generic controller.
- `dynamic_reconfigure` and Bayesian tuning were explored, but fully robust automated tuning was not claimed as solved.
- Many script filenames still reflect the original PID-experiment history even though the package itself was renamed to `balance_robot_control`.
