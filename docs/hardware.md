# Hardware

## Overview

This project combines a physical two-wheeled self-balancing robot with a separate ROS simulation and integration workflow.

The public hardware story in this repository is intentionally based only on confirmed evidence from:

- Arduino firmware
- URDF and CAD/mesh files
- the recovered wiring block diagram
- curated photos from the physical robot

![Internal hardware photo](../media/hardware/robot_open_front.jpg)

## Confirmed Hardware Summary

- Arduino Mega 2560 for low-level physical control
- BNO055 IMU for balancing feedback
- ODrive 3.6 motor controller
- RC receiver using PWM input on Arduino
- wheel motors with hall-sensor-related evidence in the wiring sketch
- a camera or depth-sensor head mounted on the upper structure
- a battery-backed onboard power distribution chain

## Hardware Docs Map

- [hardware_bom.md](hardware_bom.md): confirmed components and roles
- [hardware_power_and_io.md](hardware_power_and_io.md): power and signal flow centered on the recovered block diagram
- [hardware_layout.md](hardware_layout.md): physical placement and interpretation of the internal layout
- [hardware_gallery.md](hardware_gallery.md): image-based hardware walkthrough

## Notes

- This section is written conservatively for portfolio use.
- Exact component names are only stated when they are directly supported by code, CAD, or visible evidence.
- The camera module is described as a `depth camera / sensor head` unless a specific model is confirmed in the public-facing documentation.
