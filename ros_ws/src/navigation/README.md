# navigation

This package holds the navigation stack configuration used on top of the balancing simulation.

The most important idea in this package is that `move_base` does **not** publish directly to the robot base command topic. It is remapped to `/before_vel`, which is then consumed by the balancing controller.

That makes the navigation stack compatible with a two-wheeled self-balancing robot instead of a standard differential-drive base.

## Core Design

Normal mobile-robot flow:

`move_base -> /cmd_vel`

This project:

`move_base -> /before_vel -> balance_robot_control -> /cmd_vel`

That is the key architectural reason this package matters.

## Launch Files

### [`launch/move_base.launch`](launch/move_base.launch)

Central navigation launch.

What it does:

- starts `move_base`
- loads costmap and DWA planner parameters
- remaps `cmd_vel` to `/before_vel`
- optionally forces forward-only motion

If you want to understand how navigation was made compatible with the balance controller, this is the first file to read.

### [`launch/amcl.launch`](launch/amcl.launch)

Localization setup for map-based navigation.

What it configures:

- AMCL particle counts
- scan remap
- laser model parameters
- odom model parameters
- initial pose arguments

### [`launch/robot_navigation_lidar.launch`](launch/robot_navigation_lidar.launch)

LiDAR navigation workflow.

What it starts:

- robot bringup
- `map_server`
- AMCL
- `move_base`
- RViz

### [`launch/robot_navigation_depth.launch`](launch/robot_navigation_depth.launch)

Depth navigation workflow.

What it starts:

- depth bringup
- `map_server`
- AMCL
- `move_base`
- RViz

Important note:

- this file assumes a usable `scan` topic already exists
- the depth-to-scan projection is provided one layer above, by `balance_robot_workflows/launch/robot_navigation_depth.launch`

### [`launch/turtlebot3_navigation.launch`](launch/turtlebot3_navigation.launch)

Reference launch inherited from the earlier TurtleBot3-based navigation exploration.

This file is useful historically because it shows the baseline from which the balancing-robot navigation stack was adapted.

## Supporting Assets

### `maps/`

Contains saved navigation maps such as:

- `a1_map`
- `map`
- `mymap`
- `my_map`
- `my_map_10`

These are experiment artifacts from repeated mapping and navigation trials.

### `param/`

Contains the main planner and localization configuration:

- `costmap_common_params.yaml`
- `local_costmap_params.yaml`
- `global_costmap_params.yaml`
- `move_base_params.yaml`
- `dwa_local_planner_params.yaml`

These files define footprint, inflation behavior, DWA velocity limits, goal tolerances, and general navigation tuning.

### [`rtab_hint/rtabmap_launch_notes.txt`](rtab_hint/rtabmap_launch_notes.txt)

Recovered scratch notes for RTAB-Map launch options.

This is not a polished launch file. It is better interpreted as a working note that preserves command-line experiments for RGB-D SLAM.

## Relationship to the Rest of the Workspace

- `balance_robot_workflows` chooses the workflow
- `navigation` provides map/localization/planning pieces
- `balance_robot_control` adapts navigation output to a balancing robot

Without `balance_robot_control`, this package would behave like a more standard mobile-robot navigation stack. With `balance_robot_control`, it becomes part of a balance-aware simulation system.
