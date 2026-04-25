# Hardware Power And IO

## Primary Source Diagram

![Recovered wiring block diagram](../media/process/source_wiring_block_diagram.jpg)

This recovered block diagram is the most accurate public source for the robot's intended power and signal architecture. It is the reference image that should be used when explaining how the receiver, Arduino Mega, IMU, ODrive, motors, converters, onboard PC, camera, mini Arduino, and relay module were meant to connect.

## Clean Public Diagram

![Clean power and IO overview](../media/diagrams/hardware_power_io_overview.svg)

The SVG in this repository is a cleaned public redraw based on the recovered block diagram above.

## Secondary Planning Sketch

![Hand-drawn power and signal sketch](../media/hardware/wiring_hand_sketch.jpg)

## Interpreted Power Flow

The clean diagram is the official public-facing summary, but `source_wiring_block_diagram.jpg` is the primary source evidence. The hand sketch is kept only as a secondary planning artifact. A recovered circuit-planning deck also informed the redraw, but that raw deck is treated only as background source material and is not published as a primary artifact.

Both the diagram and sketch are high-level system views rather than full electrical schematics.

Most defensible interpretation:

1. A `36V battery` feeds the main system bus.
2. The main bus goes to the motor-control side, including the `ODrive`.
3. A `36V -> 19V` converter powers the onboard `PC`.
4. A `19V -> 5V` converter powers smaller control electronics, including a `mini Arduino` and a `relay module`.

## Interpreted Signal Flow

On the left side of the recovered block diagram, the physical control chain appears as:

- hall-sensor motor -> ODrive
- ODrive -> auxiliary electronics or breadboard area
- IMU -> Arduino
- RC receiver -> Arduino

This matches the firmware architecture where Arduino runs the balancing loop while reading IMU and RC input, and then sends motor commands to ODrive.

## High-Level Architecture

```text
36V Battery
  -> Main bus
    -> ODrive -> wheel motors
    -> 36V->19V converter -> onboard PC
    -> 19V->5V converter -> mini Arduino / relay module

IMU -> Arduino
RC receiver -> Arduino
Arduino -> ODrive command path
```

## Evidence Limits

- `source_wiring_block_diagram.jpg` is the strongest public source for this system view, but it is still a high-level block diagram rather than a manufacturing schematic.
- The hand sketch is a secondary planning artifact and should not override the recovered block diagram when the two differ in clarity.
- The recovered circuit deck was used as redraw support; it should not be treated as a standalone public schematic.
- Wire colors, connector pinouts, and exact regulator modules should not be inferred beyond what is directly visible.
- This document is intended to explain the project system story for a portfolio reviewer, not to serve as a build guide.
