# Third-Party ROS Packages

Place external ROS dependencies here when reproducing the historical simulation,
SLAM, navigation, and camera-integration workflows.

This directory is intentionally ignored by Git:

```text
ros_ws/src/third_party/*
!ros_ws/src/third_party/README.md
```

That keeps this portfolio repository lightweight while still documenting which
external packages were expected.

## Recommended Layout

If you clone source dependencies manually, place them like this:

```text
ros_ws/src/third_party/
  OrbbecSDK_ROS1/
  rtabmap_ros/
  realsense_ros_gazebo/
  ...
```

After adding packages:

```bash
cd ros_ws
catkin_make
source devel/setup.bash
```

## Expected Dependencies

| Package | Needed for | Why it appears in this project |
| --- | --- | --- |
| `OrbbecSDK_ROS1` | Physical depth-camera integration | Driver for the Orbbec Gemini 330 used in the real robot sensor-head experiments |
| `rtabmap_ros` / `rtabmap_launch` | RGB-D SLAM | Used by depth SLAM launch files and archived real-world integration launch files |
| `depthimage_to_laserscan` | Depth navigation | Converts depth images into a 2D scan-like topic for navigation experiments |
| `realsense_ros_gazebo` | Historical depth simulation | Some simulation URDF/Xacro files still use D435-style Gazebo depth-camera assets |
| `turtlebot3_gazebo` | Gazebo house world | Several Gazebo launch files use TurtleBot3's house world as a test environment |
| `turtlebot3_description` | LiDAR mesh/reference assets | The LiDAR URDF references TurtleBot3 sensor mesh assets |
| `rosserial` / `rosserial_python` | Arduino-to-ROS bridge | Used by Arduino sketches that publish `/imu`, `/odom`, or RC-derived commands into ROS |

## Orbbec Note

The physical robot hardware story uses an Orbbec Gemini 330 depth camera. The
cleaned simulation workspace still contains older D435/Realsense-style topic
names and Gazebo assets, but those should be read as historical simulation
placeholders rather than the final physical camera choice.

In other words:

- `OrbbecSDK_ROS1` belongs to the physical camera / real-world integration path.
- `realsense_ros_gazebo` belongs to the recovered simulation depth-camera path.
- Both can be useful when reproducing the archive, but they explain different
  parts of the project history.

## What Is Not Vendored Here

Do not commit full upstream third-party package snapshots unless there is a
specific reason. Prefer one of these approaches:

- install binary ROS packages with the system package manager when available
- clone source packages into this ignored `third_party/` folder locally
- document exact versions in a separate reproduction note if a workflow depends
  on a specific upstream commit

## Related Files

- [`../../README.md`](../../README.md): ROS workspace usage guide
- [`../../../archive/ros_experiments/workspace_history.md`](../../../archive/ros_experiments/workspace_history.md): archive workspace interpretation
- [`../../../archive/ros_experiments/real_world_integration/README.md`](../../../archive/ros_experiments/real_world_integration/README.md): real-world camera, TF, SLAM, and navigation traces
