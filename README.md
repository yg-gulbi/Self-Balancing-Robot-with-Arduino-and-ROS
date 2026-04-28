# Arduino-ROS Self-Balancing Robot

End-to-end self-balancing robot project combining Arduino-based real-time control with ROS simulation, SLAM, and navigation.

Arduino Mega 2560, BNO055, ODrive 3.6, an Orbbec Gemini 330 depth camera, and a FrSky radio pair (`Taranis Q X7` transmitter + `X8R` receiver) were used for the physical balancing robot. ROS and Gazebo were used to build a simulation/control stack and to test SLAM and navigation workflows in simulation.

This repository is organized as a hiring-facing portfolio. It separates verified outcomes, reusable code, hardware evidence, and legacy experiments instead of presenting every old workspace as if it were a final deliverable.

<p align="center">
  <img src="media/hero/physical_balance_hallway.gif" alt="Physical hallway balancing demo" width="720">
</p>

## What Was Actually Completed

| Area | Status | Scope | Main evidence |
| --- | --- | --- | --- |
| Physical self-balancing + RC driving | Done | Real robot balancing and manual driving on Arduino | [physical_balance_controller.ino](firmware/physical_balance_controller/physical_balance_controller.ino), [physical_balance_hallway.gif](media/hero/physical_balance_hallway.gif) |
| ROS/Gazebo balance simulation | Done | Simulated two-wheeled balancing robot with custom control nodes | [ros_ws/src/robot_controll](ros_ws/src/robot_controll), [ros_ws/src/balance_robot_gazebo](ros_ws/src/balance_robot_gazebo) |
| SLAM in simulation | Done | SLAM-related simulation workflow and launch composition | [ros_ws/src/robot_ability](ros_ws/src/robot_ability), [docs/software_architecture.md](docs/software_architecture.md) |
| Navigation in simulation | Done | `move_base -> /before_vel -> balance controller -> /cmd_vel` pipeline | [ros_ws/src/navigation](ros_ws/src/navigation), [docs/software_architecture.md](docs/software_architecture.md) |
| RC-to-ROS bridge testing | Done | Arduino bridge that published command inputs into ROS | [rc_to_ros_cmd_vel_bridge.ino](firmware/testers/rc_to_ros_cmd_vel_bridge.ino) |
| ODrive / IMU / RC / motor subsystem tests | Done | Subsystem-focused bring-up and tuning work | [docs/experiments.md](docs/experiments.md), [docs/development_process.md](docs/development_process.md), [robot_open_front.png](media/hardware/robot_open_front.png), [hardware_power_io_overview.svg](media/diagrams/hardware_power_io_overview.svg) |
| Real-world ROS SLAM/navigation integration | Partial | Launch and integration experiments exist, but end-to-end autonomous physical navigation is not claimed | [archive/legacy_code/real_world_integration](archive/legacy_code/real_world_integration), [docs/results_and_limitations.md](docs/results_and_limitations.md) |

## System Architecture

Three layers were explored in this project:

1. Physical robot control: FrSky Taranis Q X7 -> FrSky X8R -> Arduino Mega 2560 -> BNO055 + ODrive 3.6 -> balancing and driving.
2. ROS simulation: teleop or `move_base` -> `/before_vel` -> balance controller -> `/cmd_vel` -> Gazebo robot.
3. Real-world integration experiments: Orbbec Gemini 330 camera/SLAM/navigation launch composition and RC-to-ROS bridging for system-level testing.

More detail is documented in [docs/software_architecture.md](docs/software_architecture.md).

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

- [firmware/physical_balance_controller](firmware/physical_balance_controller/README.md): main physical robot controller
- [firmware/testers](firmware/testers/README.md): tester and bridge Arduino sketches, including hall-sensor, motor-current, ODrive receiver, and receiver PWM tests
- [ros_ws/src](ros_ws/src): curated ROS packages for simulation and control
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
roslaunch balance_robot_bringup robot_remote_lidar.launch
```

For navigation-related simulation experiments:

```bash
roslaunch robot_ability robot_navigation_lidar.launch
```

Before building, install the required third-party packages listed in [docs/setup.md](docs/setup.md) and place them under `ros_ws/src/third_party/`.
