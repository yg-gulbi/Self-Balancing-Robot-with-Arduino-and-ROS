# balance_robot_workflows

This package is the workflow-orchestration layer for the simulation workspace.
It does not own the balancing equation itself. Instead, it decides which
combination of robot model, sensor setup, and higher-level stack should run.

In practice, it answers questions like:

- "spawn the balancing robot with LiDAR and run normal control"
- "spawn the balancing robot with a depth camera and run navigation"
- "spawn the depth model and launch RTAB-Map SLAM"
- "start the PID-tuning workflow instead of the normal driving workflow"

## Package Role

- bundles lower-level packages into reusable launch entrypoints
- chooses among `LiDAR`, `depth`, and controller-tuning scenarios
- keeps control, navigation, and SLAM flows separate by use case
- exposes the main entrypoints for control, navigation, SLAM, and tuning

## Launch Files

### [`launch/robot_control_lidar.launch`](launch/robot_control_lidar.launch)

Starts the LiDAR-based balancing simulation by including the Gazebo launch from `robot_gazebo`.

This is the simplest "controller only" LiDAR entrypoint.

### [`launch/robot_control_depth.launch`](launch/robot_control_depth.launch)

Starts the depth-camera-based balancing simulation by including the Gazebo depth launch.

This is the depth counterpart to the LiDAR control-only workflow.

### [`launch/robot_control_lidar_pid_tuning.launch`](launch/robot_control_lidar_pid_tuning.launch)

Starts the LiDAR tuning setup and also launches `rqt_reconfigure`.

This is the manual tuning entrypoint when changing PID values live.

### [`launch/robot_navigation_lidar.launch`](launch/robot_navigation_lidar.launch)

Starts:

- Gazebo LiDAR simulation
- the balancing controller
- the navigation stack

This is the main LiDAR navigation integration launch.

### [`launch/robot_navigation_depth.launch`](launch/robot_navigation_depth.launch)

Starts:

- Gazebo depth simulation
- `depthimage_to_laserscan`
- the navigation stack

Important note:

- the navigation package itself expects a `scan` topic
- this launch is where depth-camera data is projected into a 2D laser-like scan before navigation uses it

### [`launch/robot_slam_depth.launch`](launch/robot_slam_depth.launch)

Starts the depth-camera SLAM flow in simulation.

Main pieces:

- Gazebo depth simulation
- `joint_state_publisher`
- `robot_state_publisher`
- RTAB-Map launch

This is the main RGB-D SLAM simulation entrypoint in the cleaned repository.

## How It Relates to Other Packages

- `balance_robot_workflows` decides **which workflow** to run.
- `balance_robot_control` contains the actual balancing-controller logic.
- `robot_gazebo` starts the correct simulation world and robot model.
- `navigation` contains `move_base`, AMCL, maps, and planner configs.

So the package split is:

- `balance_robot_workflows`: orchestration
- `balance_robot_control`: controller implementation
- `navigation`: planner/localization stack

## Why This Package Exists

The original archive had many launch files scattered across several packages and workspaces. This package keeps the cleaned repository easier to read by giving each common workflow a named entrypoint.

The name `balance_robot_workflows` was chosen because this package is really a
scenario selector. It is where the repo decides whether the balancing robot is
spawned with LiDAR, with a depth camera, for controller-only driving, for
manual PID tuning, for navigation, or for depth SLAM.
