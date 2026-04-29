# Arduino-ROS Self-Balancing Robot

End-to-end self-balancing robot project combining Arduino-based real-time control with ROS simulation, SLAM, and navigation.

Arduino Mega 2560, BNO055, ODrive 3.6, an Orbbec Gemini 330 depth camera, and a FrSky radio pair (`Taranis Q X7` transmitter + `X8R` receiver) were used for the physical balancing robot. ROS and Gazebo were used to build a simulation/control stack and to test SLAM and navigation workflows in simulation.

This repository is organized as a hiring-facing portfolio. It separates verified outcomes, reusable code, hardware evidence, and legacy experiments instead of presenting every old workspace as if it were a final deliverable.

<p align="center">
  <img src="media/hero/physical_balance_hallway.gif" alt="Physical hallway balancing demo" width="720">
</p>

## Control Loop Spotlight

<p align="center">
  <img src="media/diagrams/physical_balance_control_loop.png" alt="Physical balance control loop" width="860">
</p>

This diagram is the fastest way to understand the hardest technical result in the project: the Arduino closes the balance loop locally, blending RC intent, IMU tilt and yaw feedback, wheel-speed feedback, and ODrive current control into one safety-gated actuator path.

<table>
  <tr>
    <td width="33%" valign="top">
      <strong>Balance</strong><br>
      Body angle and body angular velocity generate the main stabilizing current term that keeps the robot upright.
    </td>
    <td width="33%" valign="top">
      <strong>Motion</strong><br>
      RC throttle becomes a target speed, then a speed-correction loop shifts the balance point so the robot can drive without collapsing.
    </td>
    <td width="33%" valign="top">
      <strong>Safety</strong><br>
      PWM filtering, engage persistence, tilt cutoff, and current limiting prevent noisy inputs or bad states from being passed straight to the motors.
    </td>
  </tr>
</table>

See [control_algorithm.md](firmware/physical_balance_controller/control_algorithm.md) for the full control breakdown and [project_troubleshooting_summary.md](docs/project_troubleshooting_summary.md) for the engineering problems that led to this structure.

## System At A Glance

The two system diagrams below explain the project faster than a long paragraph: one shows who controls what, and the other shows how the hardware is wired and powered.

<table>
  <tr>
    <td width="50%">
      <img src="media/diagrams/signal%20controll_diagram.png" alt="Signal / Control Diagram" width="100%">
    </td>
    <td width="50%">
      <img src="media/diagrams/Wiring%20Diagram.png" alt="Wiring Diagram" width="100%">
    </td>
  </tr>
  <tr>
    <td valign="top">
      <strong>Signal / Control Diagram</strong><br>
      Shows the control split among the receiver, IMU, Arduino Mega, ODrive, PC, and camera, including how autonomy commands and balancing logic were separated.
    </td>
    <td valign="top">
      <strong>Wiring Diagram</strong><br>
      Shows the physical 36V power chain, PC rail, 5V control rail, motor phases, hall signals, IMU wiring, RC receiver wiring, and relay-side control path.
    </td>
  </tr>
</table>

## What Was Actually Completed

| Area | Status | Scope | Main evidence |
| --- | --- | --- | --- |
| Physical self-balancing + RC driving | Done | Real robot balancing and manual driving on Arduino | [physical_balance_controller.ino](firmware/physical_balance_controller/physical_balance_controller.ino), [control_algorithm.md](firmware/physical_balance_controller/control_algorithm.md), [physical_balance_hallway.gif](media/hero/physical_balance_hallway.gif) |
| ROS/Gazebo balance simulation | Done | Simulated two-wheeled balancing robot with custom control nodes | [ros_ws/src/balance_robot_control](ros_ws/src/balance_robot_control), [ros_ws/src/balance_robot_gazebo](ros_ws/src/balance_robot_gazebo) |
| SLAM in simulation | Done | SLAM-related simulation workflow and launch composition | [ros_ws/src/balance_robot_workflows](ros_ws/src/balance_robot_workflows), [docs/software_architecture.md](docs/software_architecture.md) |
| Navigation in simulation | Done | `move_base -> /before_vel -> balance controller -> /cmd_vel` pipeline | [ros_ws/src/navigation](ros_ws/src/navigation), [docs/software_architecture.md](docs/software_architecture.md) |
| RC-to-ROS bridge testing | Done | Arduino bridge that published command inputs into ROS | [rc_to_ros_cmd_vel_bridge.ino](firmware/testers/rc_to_ros_cmd_vel_bridge.ino) |
| ODrive / IMU / RC / motor subsystem tests | Done | Subsystem-focused bring-up and tuning work | [docs/experiments.md](docs/experiments.md), [docs/development_process.md](docs/development_process.md), [robot_open_front.png](media/hardware/robot_open_front.png), [Wiring Diagram.png](<media/diagrams/Wiring Diagram.png>) |
| Real-world ROS SLAM/navigation integration | Partial | Launch and integration experiments exist, but end-to-end autonomous physical navigation is not claimed | [archive/legacy_code/real_world_integration](archive/legacy_code/real_world_integration), [docs/results_and_limitations.md](docs/results_and_limitations.md) |

## Arduino-To-ROS Code Evidence

The main physical balancing firmware keeps the real-time balance loop on Arduino, but the repository does include Arduino sketches that publish data into ROS through `rosserial`.

| Arduino sketch | ROS direction | Published topic or message | Why it matters |
| --- | --- | --- | --- |
| [physical_balance_controller_ros.ino](firmware/physical_balance_controller_ros/physical_balance_controller_ros.ino) | Arduino -> ROS | `sensor_msgs/Imu` on `/imu`, `nav_msgs/Odometry` on `/odom` | ROS-enabled version of the physical balance controller, adapted from the legacy publisher logic |
| [rc_to_ros_cmd_vel_bridge.ino](firmware/testers/rc_to_ros_cmd_vel_bridge.ino) | Arduino -> ROS | `geometry_msgs/Twist` on `cmd_vel` | Converts FrSky receiver PWM into ROS motion commands for bridge testing |
| [experimental_balance_controller_imu_ros.ino](archive/legacy_firmware/experimental_balance_controller_imu_ros.ino) | Arduino -> ROS | `sensor_msgs/Imu` on `/imu` | Experimental balance firmware that publishes BNO055 IMU data |
| [legacy_balance_controller.ino](archive/legacy_firmware/legacy_balance_controller.ino) | Arduino -> ROS trace | `sensor_msgs/Imu` on `/imu`, `nav_msgs/Odometry` on `/odom` publisher code | Legacy state-publishing functions preserved for ROS-side integration, with some calls commented in the archived sketch |

This is why the project title uses `Arduino-ROS`: Arduino handled the physical robot and bridge-side publishing, while ROS/Gazebo handled simulation, visualization, SLAM, navigation, and higher-level integration experiments.

## System Architecture

Three layers were explored in this project:

1. Physical robot control: FrSky Taranis Q X7 -> FrSky X8R -> Arduino Mega 2560 -> BNO055 + ODrive 3.6 -> balancing and driving.
2. ROS simulation: teleop or `move_base` -> `/before_vel` -> balance controller -> `/cmd_vel` -> Gazebo robot.
3. Real-world integration experiments: Orbbec Gemini 330 camera/SLAM/navigation launch composition and RC-to-ROS bridging for system-level testing.

If you want the fastest visual summary:

- [Signal / Control Diagram](<media/diagrams/signal controll_diagram.png>): control responsibility split across Arduino, ODrive, PC, and camera
- [Wiring Diagram](<media/diagrams/Wiring Diagram.png>): physical robot power, IO, and device-connection overview

More detail is documented in [docs/software_architecture.md](docs/software_architecture.md).

- [ROS workspace breakdown](docs/ros_workspaces.md): explains what the original `simul_br_ws` and `real_br_ws_intel` workspaces meant, and how their roles map into this cleaned repository
- [ros_ws/README.md](ros_ws/README.md): quickest entry point for the curated ROS workspace, launch scenarios, and package map
- [Simulation controller package guide](ros_ws/src/balance_robot_control/README.md): file-by-file breakdown of the balancing controller package
- [Real-world ROS integration archive](archive/legacy_code/real_world_integration/README.md): camera, TF, RViz, and partial physical SLAM/navigation integration notes

## Key Demos

Curated public assets:

- [Physical hallway balancing GIF](media/hero/physical_balance_hallway.gif)
- [Physical obstacle-course balancing GIF](media/demos/physical_balance_obstacle_course.gif)
- [Robot-focused hallway still](media/demos/hallway_robot_only.jpg)
- [Original full-length clip plan](media/links.md)

## Hardware Snapshot

The physical robot is backed by real hardware evidence, not code alone.

<p align="center">
  <img src="media/hardware/robot_open_front.png" alt="Internal hardware photo" width="540">
</p>

- Arduino Mega 2560 main controller
- BNO055 IMU
- ODrive 3.6 motor controller
- FrSky 2.4GHz Taranis Q X7 transmitter + FrSky X8R receiver
- Orbbec Gemini 330 depth camera
- dual 36V hall-sensor BLDC hub motors
- 36V battery pack with 36V -> 19V and 19V -> 5V conversion
- onboard mini PC, auxiliary Arduino, and relay module

- [Hardware overview](docs/hardware.md)
- [Hardware BOM](docs/hardware_bom.md): exact models plus best-effort identifications for the remaining hardware
- [Power and IO interpretation](docs/hardware_power_and_io.md)
- [Hardware layout](docs/hardware_layout.md)
- [Hardware gallery](docs/hardware_gallery.md)

## Build Process

Recovered project-process material has been summarized into public-safe documentation instead of copying raw presentation decks, office documents, chat exports, or large original videos into the repository.

<p align="center">
  <img src="media/process/catia_internal_assembly_views.jpg" alt="CATIA assembly evidence" width="31%">
  <img src="media/process/wheel_bench_test.jpg" alt="Wheel bench test" width="31%">
  <img src="media/process/tethered_driving_practice.jpg" alt="Tethered driving practice" width="31%">
</p>

- [Build story](docs/build_story.md): end-to-end narrative from concept, CAD, wiring, bench testing, embedded control, tethered practice, and ROS simulation to verified results
- [Development process](docs/development_process.md): build timeline, component bring-up, trial-and-error lessons, and final public claims
- [Project troubleshooting summary](docs/project_troubleshooting_summary.md): review-focused summary of receiver noise, ODrive firmware troubleshooting, balance tuning, ROS integration limits, and remaining verification questions
- [Research and design decisions](docs/research_and_design_decisions.md): CAN, ODrive, Orbbec Gemini 330, SLAM, FrSky receiver-noise, and ROS autorun research boundaries
- [Experiment summary](docs/experiments.md): curated implementation and integration evidence

## Repository Map

```text
README.md
media/
docs/
firmware/
ros_ws/
archive/
```

- [firmware/physical_balance_controller](firmware/physical_balance_controller/README.md): main physical robot controller and [control algorithm structure](firmware/physical_balance_controller/control_algorithm.md)
- [firmware/testers](firmware/testers/README.md): tester and bridge Arduino sketches, including hall-sensor, motor-current, ODrive receiver, and receiver PWM tests
- [ros_ws/src](ros_ws/src): curated ROS packages for simulation and control
- [ros_ws/README.md](ros_ws/README.md): workspace-level guide for build, launches, package roles, and name mapping
- [docs/ros_workspaces.md](docs/ros_workspaces.md): detailed explanation of the historical ROS workspaces and their cleaned-repo mapping
- [ros_ws/src/balance_robot_control/README.md](ros_ws/src/balance_robot_control/README.md): active balancing-controller package, file by file
- [ros_ws/src/balance_robot_workflows/README.md](ros_ws/src/balance_robot_workflows/README.md): high-level launch composition package
- [ros_ws/src/navigation/README.md](ros_ws/src/navigation/README.md): navigation stack, maps, and `/before_vel` remap explanation
- [archive](archive): legacy firmware, old PID experiments, real-world integration traces, and raw reference materials
- [media](media/README.md): curated portfolio-safe photos, GIFs, and external media plan
- [docs/build_story.md](docs/build_story.md): visual end-to-end build narrative
- [docs/development_process.md](docs/development_process.md): public-safe summary of recovered build-process material

## Limitations

- End-to-end ROS autonomous navigation on the physical robot is not claimed in this repository.
- Some legacy evidence still exists as recovered archive material rather than polished reproducible demos.
- Third-party ROS dependencies are intentionally not vendored here to keep the repository lightweight.
- Original MP4 clips are not stored in the repo. Use [media/links.md](media/links.md) to track external uploads.
- Raw PPTX, DOCX, HWPX, datasheet, and chat-log artifacts are summarized rather than published in the main GitHub tree.

## Quick Start

Example simulation workflow:

```bash
cd ros_ws
catkin_make
source devel/setup.bash
roslaunch robot_bringup robot_remote_lidar.launch
```

For navigation-related simulation experiments:

```bash
roslaunch balance_robot_workflows robot_navigation_lidar.launch
```

Before building, install the required third-party packages listed in [docs/setup.md](docs/setup.md) and place them under `ros_ws/src/third_party/`.
