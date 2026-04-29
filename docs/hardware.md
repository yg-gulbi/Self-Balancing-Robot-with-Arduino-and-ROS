# Hardware

## Overview

This project combines a physical two-wheeled self-balancing robot with a separate ROS simulation and integration workflow.

The public hardware story in this repository is intentionally based only on confirmed evidence from:

- Arduino firmware
- URDF and CAD/mesh files
- the Wiring Diagram
- curated photos from the physical robot

<table>
  <tr>
    <td width="38%">
      <img src="../media/hardware/robot_open_front.png" alt="Internal hardware photo" width="100%">
    </td>
    <td width="62%">
      <img src="../media/diagrams/Wiring%20Diagram.png" alt="Wiring Diagram" width="100%">
    </td>
  </tr>
  <tr>
    <td valign="top">
      <strong>Physical layout evidence</strong><br>
      Shows the real robot chassis, sensor head, wiring bay, and compute/battery packaging.
    </td>
    <td valign="top">
      <strong>Hardware-system summary</strong><br>
      Makes the power chain, device wiring, hall/IMU/RC signals, and auxiliary relay path readable at a glance.
    </td>
  </tr>
</table>

The internal photo shows what the robot physically looked like, while the Wiring Diagram makes the device relationships readable: battery, DC-DC converters, Intel mini PC, Gemini 330 camera, Arduino Mega, BNO055, ODrive, hub motor, auxiliary Arduino, and relay-side control path.

## Confirmed Hardware Summary

- Arduino Mega 2560 for low-level physical control
- BNO055 IMU for balancing feedback
- ODrive 3.6 motor controller
- Orbbec Gemini 330 depth camera mounted on the upper sensor head
- FrSky X8R receiver providing PWM input to Arduino
- FrSky 2.4GHz Taranis Q X7 handheld controller paired with the receiver
- a battery-backed onboard power distribution chain

## Best-Effort Hardware Identification

The remaining parts can be described confidently at the subsystem level even though some exact part numbers were not retained:

- dual hall-sensor BLDC wheel motors with roughly 165 mm wheels, likely hoverboard-type hub motors
- a 36V battery pack feeding the motor and compute power chain
- a 36V -> 19V converter for the onboard PC
- a 19V -> 5V converter for lower-voltage control electronics
- a 19V onboard mini PC, likely an x86 mini-PC or NUC-class device
- an auxiliary Arduino Mini/Nano-class board
- a relay or switching module for auxiliary power/control work
- a 3D-printed body, mast, and upper sensor-head mount on a metal base plate

## Hardware Docs Map

- [hardware_bom.md](hardware_bom.md): exact models plus best-effort identifications for the full physical hardware stack
- [hardware_power_and_io.md](hardware_power_and_io.md): power and signal flow centered on the Wiring Diagram
- [Wiring Diagram](<../media/diagrams/Wiring Diagram.png>): the fastest hardware-system summary in one image
- [hardware_layout.md](hardware_layout.md): physical placement and interpretation of the internal layout
- [hardware_gallery.md](hardware_gallery.md): image-based hardware walkthrough

## Notes

- This section is written conservatively for portfolio use.
- Exact component names are stated when they are supported by code, launch files, recovered project notes, hardware photos, or direct project confirmation.
- The detailed split between exact models and best-effort identifications is maintained in [hardware_bom.md](hardware_bom.md).
