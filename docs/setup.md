# Setup

## Repository Layout

- `firmware/`: Arduino code
- `ros_ws/`: curated ROS workspace
- `archive/`: legacy code and raw reference materials

## ROS Dependencies

This repository intentionally does not vendor third-party packages. Place required external packages under `ros_ws/src/third_party/`.

Expected dependencies from the recovered archive:

- `rtabmap_ros`
- `realsense_ros_gazebo`
- `rosserial`

Depending on which historical workflow you want to reproduce, additional camera drivers or sensor packages may also be needed.

## Build

```bash
cd ros_ws
catkin_make
source devel/setup.bash
```

## Example Launches

Manual or remote simulation workflow:

```bash
roslaunch robot_bringup robot_remote_lidar.launch
```

Simulation navigation workflow:

```bash
roslaunch robot_ability robot_navigation_lidar.launch
```

## Important Notes

- The cleaned repository preserves the main custom packages, not every third-party dependency snapshot.
- Some legacy launch files are intentionally kept under `archive/legacy_code/` because they represent integration history rather than the main reusable stack.
- Real-world physical navigation should be treated as an integration experiment, not a verified final capability.
- Some folder names differ from the actual ROS package names. For example, the folder `balance_robot_bringup/` contains the ROS package named `robot_bringup`.
