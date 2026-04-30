# Real-World Integration

This folder stores recovered ROS-side integration traces from the physical robot workflow, especially the work that originally lived in the `real_br_ws_intel` workspace.

These files are important because they show what the ROS layer was trying to do on the real platform:

1. receive depth-camera data on the onboard PC,
2. consume robot-state topics such as `/imu` and `/odom`,
3. publish TF and robot-model information for visualization,
4. attempt RTAB-Map SLAM and navigation-oriented bring-up.

They should still be read together with [`docs/results-and-limitations.md`](../../../docs/results-and-limitations.md) so this folder is not mistaken for a fully finished autonomous physical navigation result.

## What This Folder Represents

This is not the low-level physical balancing controller.

Instead, it is the ROS integration layer that sat on top of the physical robot:

- Orbbec depth-camera data into ROS
- robot state from Arduino/ROS bridging into `/imu` and `/odom`
- RViz/TF visualization support
- RTAB-Map launch composition
- partial navigation-front-end experiments

## What Was Actually Achieved

- ROS-Arduino communication was achieved in the broader project.
- depth-camera and robot-state topics were integrated on the ROS side.
- visualization and TF adaptation for the physical robot were implemented.
- RTAB-Map SLAM launch composition was attempted.

## What Was Not Finished

- end-to-end autonomous physical SLAM was not finished here
- end-to-end autonomous physical navigation was not finished here

That distinction matters when reading filenames such as `robot_navigation.launch`, because the real physical workflow was still incomplete.

## File Guide

### [`robot_slam.launch`](robot_slam.launch)

Recovered SLAM-oriented real-world integration launch.

What it starts:

- Orbbec Gemini 330 camera driver
- RTAB-Map
- robot-description publication
- custom TF helpers

What it assumes already exists:

- `/imu`
- `/odom`

Those topics were expected from the physical robot side, not generated here.

### [`robot_navigation.launch`](robot_navigation.launch)

Recovered depth-camera navigation-front-end launch from the original `real_br_ws_intel` workspace.

What it actually does:

- starts the Orbbec camera
- runs `depthimage_to_laserscan`

What it does **not** do:

- it does not launch `move_base`
- it does not launch `amcl`
- it does not form a complete autonomous navigation stack by itself

That makes this file better interpreted as a sensor-preparation launch for navigation experiments rather than proof of completed physical navigation.

### [`robot_navigation_lidar.launch`](robot_navigation_lidar.launch)

Older navigation-oriented launch composition trace kept as recovered archive material.

Compared with `robot_navigation.launch`, this file is closer to a normal map-server/AMCL/`move_base` navigation flow, but it should still be treated as integration history rather than a verified final robot demo.

### [`move_base.launch`](move_base.launch)

Recovered `move_base` configuration trace from the physical-integration side.

Notably, this recovered file uses a direct `cmd_vel` default, which is a useful contrast with the cleaned simulation stack where `move_base` is remapped to `/before_vel`.

### [`robot_description_rviz.launch`](robot_description_rviz.launch)

Recovered RViz-focused launch that loads the robot model, TF helper nodes, joint state publication, robot state publication, and RViz itself.

This shows that the ROS integration effort emphasized making the physical robot interpretable inside RViz, not only launching camera nodes.

### [`imu_tf.py`](imu_tf.py)

Converts IMU orientation into a TF that keeps only pitch between `base_footprint` and `base_link`.

Interpretation:

- the physical robot was a balancing platform
- the ROS visualization stack wanted a simplified body-tilt representation that highlighted pitch behavior

### [`odom_tf.py`](odom_tf.py)

Broadcasts a TF from `odom` to `camera_link` from incoming odometry messages.

Interpretation:

- the real-world ROS stack was trying to anchor camera-based SLAM or visualization to the robot odometry stream

### [`visual_odom_tf.py`](visual_odom_tf.py)

Alternative TF broadcaster from `odom1` to `camera_link`.

Interpretation:

- this looks like an experiment around alternate odometry sources or frame naming
- it reinforces that the workspace was actively adjusting frame conventions during integration

### [`launch_service.py`](launch_service.py)

Simple service wrapper around `roslaunch`.

What it provides:

- start SLAM launch
- start navigation launch
- stop current launch

Interpretation:

- this was an operational convenience layer for toggling larger ROS workflows without manually retyping long commands

## Where `/imu` and `/odom` Came From

The files in this folder subscribe to `/imu` and `/odom`, but they do not publish those topics.

Representative traces of that topic publishing exist elsewhere in the repository:

- [`archive/arduino_firmware/legacy_balance_controller.ino`](../../arduino_firmware/legacy_balance_controller.ino)
- [`archive/arduino_firmware/experimental_balance_controller_imu_ros.ino`](../../arduino_firmware/experimental_balance_controller_imu_ros.ino)
- [`firmware/testers/rc_to_ros_cmd_vel_bridge.ino`](../../../firmware/testers/rc_to_ros_cmd_vel_bridge.ino)

That is the architectural split:

- Arduino / firmware side: publish or bridge physical robot state and control-related topics
- ROS real-world side: consume those topics for visualization, SLAM, and higher-level experiments

## Important Workspace Interpretation

The original `real_br_ws_intel` workspace also contained:

- `robot_description`
- `robot_bringup`
- `robot_gazebo`
- large world assets

That mixed structure suggests a historical integration sandbox rather than a minimal production workspace. In other words, it was a practical place to test camera, TF, visualization, and navigation ideas, not a final polished deployment package.
