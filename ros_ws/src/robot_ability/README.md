# robot_ability

This package does not contain the controller logic itself. Instead, it acts as the high-level launch composition layer for the simulation workspace.

In practice, it answers questions like:

- "run the LiDAR simulation"
- "run the depth simulation"
- "run the navigation stack on the balancing robot"
- "run the depth SLAM experiment"

## Package Role

- bundles lower-level packages into reusable launch entrypoints
- keeps control, navigation, and SLAM flows separate by use case
- exposes the workflow that was originally spread across `simul_br_ws`

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

- `robot_ability` decides **which workflow** to run.
- `robot_controll` contains the actual balancing-controller logic.
- `robot_gazebo` starts the correct simulation world and robot model.
- `navigation` contains `move_base`, AMCL, maps, and planner configs.

So the package split is:

- `robot_ability`: orchestration
- `robot_controll`: controller implementation
- `navigation`: planner/localization stack

## Why This Package Exists

The original archive had many launch files scattered across several packages and workspaces. This package keeps the cleaned repository easier to read by giving each common workflow a named entrypoint.
