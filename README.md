# Arduino-ROS Self-Balancing Robot

End-to-end self-balancing robot project combining Arduino-based real-time control with a ROS/Gazebo simulation, tuning, SLAM, and navigation workflow.

The strongest verified result is the physical two-wheeled robot balancing and driving under Arduino control. ROS is used for simulation, visualization, navigation experiments, and Arduino-to-ROS bridge evidence, while real-world autonomous ROS navigation is documented as partial integration work rather than a completed claim.

<p align="center">
  <img src="media/hero/physical_balance_hallway.gif" alt="Physical hallway balancing demo" width="720">
</p>

## Core Result

<p align="center">
  <img src="media/diagrams/physical_balance_control_loop.png" alt="Physical balance control loop" width="860">
</p>

The Arduino closes the balance loop locally: RC intent, IMU tilt/yaw feedback, wheel-speed feedback, safety checks, and ODrive current commands are blended into one real-time actuator path.

| Control layer | What it does |
| --- | --- |
| Balance | Body angle and angular velocity generate the main stabilizing current term. |
| Motion | RC throttle becomes a target speed, then shifts the balance point so the robot can drive while staying upright. |
| Safety | PWM filtering, engage persistence, tilt cutoff, and current limiting prevent bad states from reaching the motors. |

Read this first: [physical balance control algorithm](firmware/physical_balance_controller/control_algorithm.md) and [troubleshooting summary](docs/project_troubleshooting_summary.md).

## Verified Scope

| Area | Status | Evidence |
| --- | --- | --- |
| Physical self-balancing and RC driving | Completed | [physical_balance_controller.ino](firmware/physical_balance_controller/physical_balance_controller.ino), [hallway demo](media/hero/physical_balance_hallway.gif) |
| ROS/Gazebo balance simulation | Completed | [balance_robot_control](ros_ws/src/balance_robot_control), [balance_robot_gazebo](ros_ws/src/balance_robot_gazebo) |
| Simulation navigation pipeline | Completed | [navigation](ros_ws/src/navigation), [balance_robot_workflows](ros_ws/src/balance_robot_workflows) |
| Simulation SLAM/navigation launch workflow | Implemented | [balance_robot_workflows](ros_ws/src/balance_robot_workflows), [software architecture](docs/software_architecture.md) |
| Arduino-to-ROS bridge tests | Completed | [rc_to_ros_cmd_vel_bridge.ino](firmware/testers/rc_to_ros_cmd_vel_bridge.ino), [physical_balance_controller_ros.ino](firmware/physical_balance_controller_ros/physical_balance_controller_ros.ino) |
| Real-world ROS SLAM/navigation | Partial | [real-world integration archive](archive/ros_experiments/real_world_integration), [results and limitations](docs/results_and_limitations.md) |

## System At A Glance

<table>
  <tr>
    <td width="50%">
      <img src="media/diagrams/signal_control_diagram.png" alt="Signal / Control Diagram" width="100%">
    </td>
    <td width="50%">
      <img src="media/diagrams/wiring_diagram.png" alt="Wiring Diagram" width="100%">
    </td>
  </tr>
  <tr>
    <td valign="top">
      <strong>Control responsibility</strong><br>
      Arduino owns the real-time balance loop. ROS-side commands are treated as higher-level inputs, not direct motor authority.
    </td>
    <td valign="top">
      <strong>Hardware wiring</strong><br>
      36V power, ODrive, Arduino, BNO055, FrSky receiver, onboard PC, Gemini 330 camera, auxiliary electronics, and motors are summarized in one view.
    </td>
  </tr>
</table>

Main physical hardware: Arduino Mega 2560, BNO055 IMU, ODrive 3.6, FrSky Taranis Q X7 + X8R, Orbbec Gemini 330, dual hall-sensor BLDC hub motors, 36V battery, onboard mini PC, DC-DC converters, auxiliary Arduino, and relay module.

For the consolidated hardware explanation, see [docs/hardware.md](docs/hardware.md).

## Arduino-To-ROS Evidence

The project title uses `Arduino-ROS` because Arduino handled the physical robot and bridge-side publishing, while ROS/Gazebo handled simulation, visualization, SLAM/navigation workflows, and integration experiments.

| Sketch | ROS role | Published data |
| --- | --- | --- |
| [physical_balance_controller_ros.ino](firmware/physical_balance_controller_ros/physical_balance_controller_ros.ino) | ROS-enabled physical controller variant | `/imu`, `/odom` |
| [rc_to_ros_cmd_vel_bridge.ino](firmware/testers/rc_to_ros_cmd_vel_bridge.ino) | RC receiver to ROS command bridge | `cmd_vel` |
| [experimental_balance_controller_imu_ros.ino](archive/arduino_firmware/experimental_balance_controller_imu_ros.ino) | Archived IMU publisher experiment | `/imu` |
| [legacy_balance_controller.ino](archive/arduino_firmware/legacy_balance_controller.ino) | Archived ROS publisher trace | `/imu`, `/odom` publisher logic |

## ROS Workspace

```bash
cd ros_ws
catkin_make
source devel/setup.bash
roslaunch robot_bringup robot_remote_lidar.launch
```

Navigation-oriented simulation workflow:

```bash
roslaunch balance_robot_workflows robot_navigation_lidar.launch
```

Use [ros_ws/README.md](ros_ws/README.md) for workspace usage and [balance_robot_control/README.md](ros_ws/src/balance_robot_control/README.md) for the controller package layout.

## Public Demos

- [Physical hallway balancing GIF](media/hero/physical_balance_hallway.gif)
- [Physical obstacle-course balancing GIF](media/demos/physical_balance_obstacle_course.gif)
- [Robot-focused hallway still](media/demos/hallway_robot_only.jpg)
- [Open-front hardware photo](media/hardware/robot_open_front.png)

Full original MP4 files are intentionally not stored in this repository; lightweight public-safe GIFs and cropped images are used instead.

## Read Next

- [Results and limitations](docs/results_and_limitations.md): clear boundary between completed work and partial integration.
- [Development process](docs/development_process.md): compact build story from CAD and wiring to bench tests, tethered practice, and ROS simulation.
- [Software architecture](docs/software_architecture.md): physical firmware, ROS simulation, and real-world integration tracks.
- [Experiments](docs/experiments.md): curated test matrix and evidence links.
- [Setup](docs/setup.md): dependencies and build expectations.
- [Archive guide](archive/README.md): historical ROS experiments, Arduino firmware variants, and raw source material.

## Limitations

- End-to-end autonomous ROS navigation on the physical balancing robot is not claimed.
- Some historical code remains in `archive/` only as evidence of earlier integration attempts.
- Third-party ROS dependencies are not vendored into this repository.
- Original raw process files, chat exports, and full-length videos are summarized or replaced with public-safe assets.
