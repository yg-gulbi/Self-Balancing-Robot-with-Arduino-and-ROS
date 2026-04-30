# ROS Workspaces: Simulation and Real-World Integration

The original project archive did not evolve as one clean ROS workspace. It split into multiple workspaces over time as balancing control, Gazebo simulation, SLAM, navigation, camera integration, and physical-robot experiments were explored in parallel.

This repository keeps that history in a cleaner form:

- `ros_ws/` is the curated simulation-focused ROS workspace.
- `archive/ros_experiments/real_world_integration/` stores representative real-robot ROS integration traces.
- `archive/arduino_firmware/` and `firmware/testers/` preserve the Arduino-to-ROS side of the bridge work.

## Original Workspace Meanings

### `simul_br_ws`

`simul_br_ws` was the main simulation workspace for the balancing robot.

Its main job was:

1. spawn the balancing robot in Gazebo,
2. run a custom balancing controller,
3. optionally feed that controller from teleop or `move_base`,
4. test SLAM and navigation on top of the balancing layer.

The most important design decision in this workspace was the command pipeline:

`move_base -> /before_vel -> balance controller -> /cmd_vel`

That means navigation did not drive the robot base directly. Navigation only produced a desired motion command, and the custom balance controller converted that request into a motion that tried to keep the robot upright.

#### `simul_br_ws` -> current repository mapping

| Original role | Current location | What it does |
| --- | --- | --- |
| balancing control nodes | [ros_ws/src/balance_robot_control](../ros_ws/src/balance_robot_control/README.md) | file-by-file balancing controller logic, tuning helpers, auto-tuning experiments |
| high-level launch composition | [ros_ws/src/balance_robot_workflows](../ros_ws/src/balance_robot_workflows/README.md) | bundles controller, Gazebo, SLAM, and navigation launch flows |
| navigation stack | [ros_ws/src/navigation](../ros_ws/src/navigation/README.md) | `map_server`, `amcl`, `move_base`, map assets, DWA config |
| Gazebo robot launch presets | `ros_ws/src/balance_robot_gazebo` | starts house/depth/lidar simulation worlds and controller entrypoints |
| URDF/Xacro robot model | `ros_ws/src/balance_robot_description` | robot body, wheels, IMU, lidar/depth model definitions |
| earlier controller variants | [archive/ros_experiments/pid_experiments](../archive/ros_experiments/pid_experiments/README.md) | older control ideas that came before the cleaned `/before_vel` structure |

### `real_br_ws_intel`

`real_br_ws_intel` was the later physical-robot ROS integration workspace.

Its goal was not low-level balancing on ROS. Instead, it attempted to:

1. bring depth-camera data into ROS from an Intel-class onboard PC,
2. subscribe to robot state information coming from the physical robot, especially `/imu` and `/odom`,
3. publish TF and robot model information for RViz and RTAB-Map,
4. try SLAM and navigation on the physical platform.

The important limitation is that this workspace represents integration attempts, not a fully verified autonomous physical navigation result.

- ROS-Arduino communication was achieved.
- camera + TF + visualization + SLAM launch composition was attempted.
- end-to-end physical SLAM/navigation success is not claimed here.

#### `real_br_ws_intel` -> current repository mapping

| Original package or role | Current location | What it means |
| --- | --- | --- |
| historical workflow-launch layer | [archive/ros_experiments/real_world_integration/robot_slam.launch](../archive/ros_experiments/real_world_integration/robot_slam.launch), [archive/ros_experiments/real_world_integration/robot_navigation.launch](../archive/ros_experiments/real_world_integration/robot_navigation.launch) | Orbbec camera bring-up, RTAB-Map, depth-to-scan conversion, partial navigation front-end |
| `robot_rviz` TF and RViz helpers | [archive/ros_experiments/real_world_integration/robot_description_rviz.launch](../archive/ros_experiments/real_world_integration/robot_description_rviz.launch), [imu_tf.py](../archive/ros_experiments/real_world_integration/imu_tf.py), [odom_tf.py](../archive/ros_experiments/real_world_integration/odom_tf.py), [visual_odom_tf.py](../archive/ros_experiments/real_world_integration/visual_odom_tf.py) | converts `/imu` and `/odom` into TF frames usable by RViz and SLAM |
| launch control helper | [archive/ros_experiments/real_world_integration/launch_service.py](../archive/ros_experiments/real_world_integration/launch_service.py) | start/stop services for SLAM and navigation launch files |
| camera driver dependency | third-party dependency, not vendored in cleaned repo | `OrbbecSDK_ROS1` publishes `/camera/depth/image_raw`, `/camera/color/image_raw`, camera info, and camera control services |
| physical robot `/imu` and `/odom` publishers | [archive/arduino_firmware/legacy_balance_controller.ino](../archive/arduino_firmware/legacy_balance_controller.ino), [experimental_balance_controller_imu_ros.ino](../archive/arduino_firmware/experimental_balance_controller_imu_ros.ino) | archived `rosserial` firmware traces that published robot-state topics into ROS |
| RC-to-ROS bridge experiment | [firmware/testers/rc_to_ros_cmd_vel_bridge.ino](../firmware/testers/rc_to_ros_cmd_vel_bridge.ino) | Arduino bridge that published ROS motion commands from RC input |

## Important Interpretation Notes

### `real_br_ws_intel` was not a full navigation workspace yet

One of the easiest ways to misunderstand the original archive is to trust filenames too literally.

For example, the recovered `robot_navigation.launch` from the real-world integration workspace does **not** launch a full `move_base` + `amcl` navigation stack. It mainly starts the Orbbec camera and `depthimage_to_laserscan`, which makes it more of a sensor front-end experiment than a complete navigation pipeline.

### ROS was consuming physical robot state, not generating it

The real-world launch files subscribe to `/imu` and `/odom`, but they do not generate those topics themselves.

That information was expected to come from the physical robot side, via Arduino and ROS bridging experiments. In other words, the ROS integration workspace was the consumer of robot state, not the low-level source of balancing state estimation.

### `real_br_ws_intel` still carried simulation-era baggage

The original workspace also contained `robot_gazebo`, robot-description world assets, and other simulation artifacts. That is useful historical evidence, but it also shows that the workspace was a mixed integration sandbox rather than a tightly scoped final product.

## Recommended Reading Order

1. [ros_ws/src/balance_robot_control/README.md](../ros_ws/src/balance_robot_control/README.md)
2. [ros_ws/src/balance_robot_workflows/README.md](../ros_ws/src/balance_robot_workflows/README.md)
3. [ros_ws/src/navigation/README.md](../ros_ws/src/navigation/README.md)
4. [archive/ros_experiments/real_world_integration/README.md](../archive/ros_experiments/real_world_integration/README.md)
5. [docs/results_and_limitations.md](results_and_limitations.md)

That sequence explains what the simulation workspace actually accomplished, what the cleaned package entrypoints are, and where the physical-robot ROS integration stopped.
