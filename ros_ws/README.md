# ROS Workspace Guide

This folder is the curated ROS workspace for the self-balancing robot project.
It is meant to explain how the simulation, balancing controller, SLAM, and
navigation launch files fit together.

The physical robot's fast balance loop runs on Arduino. In this workspace, ROS
is used for Gazebo simulation, workflow orchestration, navigation, SLAM, RViz,
and controller tuning experiments.

## Core Command Flow

The key idea is that high-level commands do not drive the balancing robot
directly.

```text
teleop or move_base
  -> /before_vel
  -> balance_robot_control
  -> /cmd_vel
  -> Gazebo robot
```

`/before_vel` is the desired motion command. The balancing controller converts
that command into a final `/cmd_vel` that respects the robot's pitch, velocity,
and steering behavior.

## Environment

This workspace targets a ROS 1 `catkin` setup, most likely Ubuntu + ROS Noetic
or a similar ROS 1 distribution.

Expected tools and packages:

- `catkin_make`
- `gazebo_ros`
- `xacro`
- `robot_state_publisher`
- `joint_state_publisher`
- `map_server`
- `amcl`
- `move_base`
- `dynamic_reconfigure`
- `rqt_reconfigure`
- `OrbbecSDK_ROS1` for physical Orbbec Gemini 330 camera integration traces
- `rtabmap_ros` or `rtabmap_launch` for depth SLAM workflows
- `depthimage_to_laserscan` for depth-camera navigation workflows
- `realsense_ros_gazebo` for historical D435-style Gazebo depth simulation assets

Third-party packages that are not part of this repository should be placed
under `src/third_party/` or installed through the normal ROS package manager.

## Build

From this folder:

```bash
cd ros_ws
catkin_make
source devel/setup.bash
```

If you open a new terminal, run `source devel/setup.bash` again before launching
anything from this workspace.

## Quick Start

Use these as the main public-facing entrypoints.

| Goal | Command | Use this when |
| --- | --- | --- |
| Show the LiDAR robot model only | `roslaunch robot_bringup robot_remote_lidar.launch` | You want to inspect URDF, TF, and robot state publishing |
| Show the depth-camera robot model only | `roslaunch robot_bringup robot_remote_depth.launch` | You want to inspect the depth-camera version of the robot |
| Run LiDAR balance simulation | `roslaunch balance_robot_workflows robot_control_lidar.launch` | You want the simplest Gazebo balancing-controller demo |
| Run depth balance simulation | `roslaunch balance_robot_workflows robot_control_depth.launch` | You want the depth-camera robot model with the balancing controller |
| Tune PID gains manually | `roslaunch balance_robot_workflows robot_control_lidar_pid_tuning.launch` | You want Gazebo plus `rqt_reconfigure` for live controller tuning |
| Run LiDAR navigation simulation | `roslaunch balance_robot_workflows robot_navigation_lidar.launch` | You want map localization, `move_base`, RViz, and balance-aware command remapping |
| Run depth navigation simulation | `roslaunch balance_robot_workflows robot_navigation_depth.launch` | You want depth data projected into a scan-like navigation pipeline |
| Run depth SLAM simulation | `roslaunch balance_robot_workflows robot_slam_depth.launch` | You want the RGB-D/RTAB-Map SLAM experiment |

## Manual Driving

For manual command testing, start a control workflow first:

```bash
roslaunch balance_robot_workflows robot_control_lidar.launch
```

Then publish desired motion into `/before_vel`. The helper script is:

```bash
rosrun balance_robot_control teleop_before_vel.py
```

The important detail is that manual teleop should target `/before_vel`, not
`/cmd_vel`. The controller owns `/cmd_vel`.

## Navigation Workflow

The navigation stack is adapted from a normal mobile-robot setup, but the output
topic is remapped.

Normal robot:

```text
move_base -> /cmd_vel
```

This balancing robot:

```text
move_base -> /before_vel -> balance_robot_control -> /cmd_vel
```

Recommended launch:

```bash
roslaunch balance_robot_workflows robot_navigation_lidar.launch
```

What this starts:

- Gazebo with the LiDAR robot model
- the balance-aware controller
- map server
- AMCL localization
- `move_base`
- RViz

In RViz, use `2D Pose Estimate` for the initial pose and `2D Nav Goal` for a
target. The planner output is routed through the balance controller before the
robot receives a final velocity command.

## SLAM Workflow

Recommended launch:

```bash
roslaunch balance_robot_workflows robot_slam_depth.launch
```

This starts the depth-camera robot workflow and RTAB-Map-related launch logic.
This path depends on external RGB-D SLAM packages, so it is the most likely
workflow to require extra package installation before it runs cleanly.

## PID Tuning Workflow

Manual tuning entrypoint:

```bash
roslaunch balance_robot_workflows robot_control_lidar_pid_tuning.launch
```

The tuning flow uses `dynamic_reconfigure`, so gains can be edited from
`rqt_reconfigure`.

The control package also keeps automated tuning experiments:

- `src/balance_robot_control/src/tuning_tools/pid_bayes_optimizer.py`
- `src/balance_robot_control/src/tuning_tools/pid_loop_test_runner.py`
- `src/balance_robot_control/src/tuning_tools/pid_reconfigure_client.py`

These scripts are evidence of repeated controller-evaluation experiments. They
are not presented as a fully solved production auto-tuning system.

## Topic Contract

| Topic | Direction | Meaning |
| --- | --- | --- |
| `/before_vel` | input to `balance_robot_control` | Desired high-level motion from teleop or navigation |
| `/cmd_vel` | output from `balance_robot_control` | Final balance-aware command sent to the simulated robot |
| `/imu` | input to controller | Simulated robot attitude and angular velocity |
| `/odom` | input to controller/navigation | Simulated odometry feedback |
| `/scan` | input to navigation | LiDAR or depth-projected scan data |
| `/map` | input to navigation/RViz | Static map from `map_server` or SLAM output |

## Package Map

| Folder | ROS package name | Role |
| --- | --- | --- |
| `src/balance_robot_bringup/` | `robot_bringup` | Robot model bringup and description launch entrypoints |
| `src/balance_robot_description/` | `robot_description` | URDF/Xacro definitions for LiDAR and depth variants |
| `src/balance_robot_gazebo/` | `robot_gazebo` | Gazebo worlds, robot spawn files, and controller launch files |
| `src/balance_robot_control/` | `balance_robot_control` | Balance-aware control nodes, PID tuning, and tuning automation helpers |
| `src/balance_robot_workflows/` | `balance_robot_workflows` | Scenario-level launch files for control, tuning, navigation, and SLAM |
| `src/navigation/` | `navigation` | `move_base`, AMCL, maps, planner parameters, and RViz configs |
| `src/third_party/` | external packages | Optional location for dependencies not committed to this repo |

## Reading Order

If you are reviewing the project on GitHub, read in this order:

1. [`src/balance_robot_workflows/README.md`](src/balance_robot_workflows/README.md)
2. [`src/balance_robot_control/README.md`](src/balance_robot_control/README.md)
3. [`src/navigation/README.md`](src/navigation/README.md)
4. [`../docs/software_architecture.md`](../docs/software_architecture.md)
5. [`../docs/results_and_limitations.md`](../docs/results_and_limitations.md)

## Troubleshooting

If a package cannot be found, source the workspace again:

```bash
source devel/setup.bash
rospack find balance_robot_control
```

If Gazebo opens but the robot does not move, check that the controller is
running and that `/before_vel` is receiving commands:

```bash
rostopic list
rostopic echo /before_vel
rostopic echo /cmd_vel
rosnode list
```

If navigation does not move the robot, confirm that `move_base` is publishing
to `/before_vel` and that `balance_robot_control` is republishing to `/cmd_vel`.

If SLAM launch fails, install or provide the missing RGB-D packages first. The
most likely missing packages are `rtabmap_ros`, `rtabmap_launch`, camera plugin
packages, or depth-to-scan conversion packages.

## Scope And Limits

- This is strongest as a simulation and integration workspace.
- The real physical balance loop was implemented on Arduino firmware, not ROS.
- Physical camera, TF, and SLAM integration traces are preserved mainly in
  [`../archive/ros_experiments/real_world_integration`](../archive/ros_experiments/real_world_integration/README.md).
- End-to-end autonomous navigation on the physical balancing robot is not
  claimed as complete in this repository.
