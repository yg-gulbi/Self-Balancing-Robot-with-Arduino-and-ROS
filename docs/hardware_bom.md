# Hardware BOM

This page is the public-facing hardware list for the physical robot. It separates exact models that are now confirmed from strong best-effort identifications for parts whose exact part number was not retained.

## GitHub-Ready Summary

- Arduino Mega 2560
- BNO055 IMU
- ODrive 3.6
- Orbbec Gemini 330 depth camera
- FrSky 2.4GHz Taranis Q X7 controller
- FrSky X8R receiver
- dual hall-sensor BLDC wheel motors, likely hoverboard-type hub motors
- 36V battery pack
- onboard mini PC powered from a 19V rail
- 36V -> 19V and 19V -> 5V DC-DC converters
- auxiliary Arduino Mini/Nano-class board
- relay module

## Exact Confirmed Components

| Component | Exact model or identification | Role | Evidence |
| --- | --- | --- | --- |
| Main controller | Arduino Mega 2560 | Main low-level controller for the physical balancing robot | `physical_balance_controller.ino` header comments and Arduino firmware archive |
| IMU | BNO055 IMU module | Body angle and gyro feedback for balancing | `physical_balance_controller.ino`, `experimental_balance_controller_imu_ros.ino` |
| Motor controller | ODrive 3.6 | Current-based dual-motor control | `physical_balance_controller.ino`, `legacy_balance_controller.ino` |
| Depth camera | Orbbec Gemini 330 | Upper sensor-head perception hardware for physical ROS integration experiments | `archive/legacy_code/real_world_integration/robot_slam.launch`, `docs/research_and_design_decisions.md`, physical sensor-head photos |
| RC transmitter | FrSky 2.4GHz Taranis Q X7 | Handheld manual controller for throttle, steering, and engage behavior | project build notes and the paired X8R receiver setup |
| RC receiver | FrSky X8R | Onboard PWM receiver feeding manual-drive commands into Arduino | `physical_balance_controller.ino`, recovered wiring/control notes, and the paired FrSky radio setup |

The transmitter is an external handheld controller, while the receiver is mounted on the robot and feeds PWM signals into the Arduino.

## Historical Camera Naming Note

Some simulation-side URDF and launch files in the repository still reference `RealSense D435` assets or `d435` topic names. Those belong to historical simulation or placeholder RGB-D workflows and should not be read as the deployed physical camera choice. The physical robot hardware documented in this BOM used an `Orbbec Gemini 330`.

## Strong Best-Effort Identifications

| Component | Best-effort identification | Why this is likely | Evidence |
| --- | --- | --- | --- |
| Wheel and motor assembly | Two 36V hall-sensor BLDC hub motors with roughly 165 mm wheels, likely hoverboard-type | The control stack uses hall-sensor feedback, the power diagram shows a 36V motor bus, the wheel radius in firmware is about `0.0825 m`, and the photos match hoverboard-style hub wheels | `physical_balance_controller.ino`, `docs/hardware_power_and_io.md`, `media/process/wheel_bench_test.jpg`, `media/hardware/robot_open_front.png` |
| Battery pack | 36V battery pack, likely lithium-ion | The Wiring Diagram and the internal robot photo both support a 36V battery feeding the system | `docs/hardware_power_and_io.md`, `media/process/Wiring Diagram.png`, `media/hardware/robot_open_front.png` |
| Power conversion chain | 36V -> 19V DC-DC converter and 19V -> 5V DC-DC converter | The Wiring Diagram explicitly shows a two-stage conversion chain for compute and control electronics | `docs/hardware_power_and_io.md`, `media/process/Wiring Diagram.png` |
| Onboard compute unit | 19V mini PC, likely an x86 mini-PC or NUC-class device | The wiring architecture dedicates a 19V rail to the `PC` block, and the internal photo shows a vented compute enclosure in the chassis | `docs/hardware_power_and_io.md`, `media/hardware/robot_open_front.png` |
| Auxiliary microcontroller | Arduino Mini/Nano-class board | The Wiring Diagram labels a `mini Arduino` on the 5V side, but the exact board variant is not readable from the public photos | `docs/hardware_power_and_io.md`, `media/process/Wiring Diagram.png` |
| Relay / switching board | 5V relay module | The 5V auxiliary branch is explicitly drawn toward a relay/power-relay block | `docs/hardware_power_and_io.md`, `media/process/Wiring Diagram.png` |
| Breadboard / distribution area | Small breadboard or wiring-distribution area | Both the source wiring diagram and the internal wiring photo show an intermediary wiring area between Arduino, IMU, and ODrive-side electronics | `docs/hardware_power_and_io.md`, `media/hardware/robot_open_front.png` |
| Chassis structure | 3D-printed body, mast, and upper sensor-head mount on a metal base plate | The process images and recovered CAD assets show a designed body shell, upper mast, and head enclosure rather than a temporary bench-only layout | `media/process/catia_internal_assembly_views.jpg`, `media/process/catia_body_design.jpg`, `media/hardware/robot_open_front.png` |

## Visual Component Evidence

- [CATIA internal assembly views](../media/process/catia_internal_assembly_views.jpg): internal packaging and chassis volume evidence.
- [Wiring Diagram](<../media/process/Wiring Diagram.png>): public system-level summary of the robot wiring and control architecture.
- [Wheel bench test](../media/process/wheel_bench_test.jpg): motor, wheel, power, and controller bench-test evidence before full robot practice.
- [Tethered driving practice](../media/process/tethered_driving_practice.jpg): safety-supported driving practice during physical tuning.

## Structure Evidence

Recovered mesh names indicate the project included modeled physical structure files for:

- robot body
- robot neck
- head parts
- a camera-case-like upper enclosure

This supports the hardware narrative that the project included both working electronics and a purpose-built physical chassis.

## Still Unverified At Model Level

- exact battery chemistry and capacity
- exact motor model number, rated power, and KV
- exact onboard PC make/model
- exact DC-DC converter modules
- exact auxiliary Arduino board variant
- exact relay module part number
