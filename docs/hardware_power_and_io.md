# Hardware Power And IO

## Primary Source Diagram

![Recovered wiring block diagram](../media/process/source_wiring_block_diagram.jpg)

This recovered block diagram is the most accurate public source for the robot's intended power and signal architecture. It is the reference image that should be used when explaining how the FrSky Taranis Q X7, FrSky X8R receiver, Arduino Mega 2560, BNO055 IMU, ODrive 3.6, wheel motors, battery, converters, onboard PC, Orbbec Gemini 330, auxiliary Arduino, and relay module were meant to connect.

## Clean Public Diagram

![Clean power and IO overview](../media/diagrams/hardware_power_io_overview.svg)

The SVG in this repository is a cleaned public redraw based on the recovered block diagram above.

## Secondary Planning Sketch

![Hand-drawn power and signal sketch](../media/hardware/wiring_hand_sketch.jpg)

## Interpreted Power Flow

The clean diagram is the official public-facing summary, but `source_wiring_block_diagram.jpg` is the primary source evidence. The hand sketch is kept only as a secondary planning artifact. A recovered circuit-planning deck also informed the redraw, but that raw deck is treated only as background source material and is not published as a primary artifact.

Both the diagram and sketch are high-level system views rather than full electrical schematics.

In practical terms, the hardware stack described here is: `FrSky Taranis Q X7 -> FrSky X8R -> Arduino Mega 2560 + BNO055 -> ODrive 3.6 -> dual wheel motors`, with a `36V battery`, `36V -> 19V` and `19V -> 5V` conversion chain, an onboard `mini PC`, an auxiliary `mini Arduino`, a `relay module`, and an `Orbbec Gemini 330` mounted on the upper sensor head.

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
- FrSky X8R receiver -> Arduino

The handheld manual controller used with that receiver was a FrSky 2.4GHz Taranis Q X7.

This matches the firmware architecture where Arduino runs the balancing loop while reading IMU and RC input, and then sends motor commands to ODrive.

## High-Level Architecture

```text
36V Battery
  -> Main bus
    -> ODrive -> wheel motors
    -> 36V->19V converter -> onboard PC
    -> 19V->5V converter -> mini Arduino / relay module

BNO055 IMU -> Arduino Mega 2560
FrSky X8R receiver -> Arduino Mega 2560
Arduino Mega 2560 -> ODrive 3.6 command path
Orbbec Gemini 330 -> onboard PC
```

## Evidence Limits

- `source_wiring_block_diagram.jpg` is the strongest public source for this system view, but it is still a high-level block diagram rather than a manufacturing schematic.
- The hand sketch is a secondary planning artifact and should not override the recovered block diagram when the two differ in clarity.
- The recovered circuit deck was used as redraw support; it should not be treated as a standalone public schematic.
- Wire colors, connector pinouts, and exact regulator modules should not be inferred beyond what is directly visible.
- This document is intended to explain the project system story for a portfolio reviewer, not to serve as a build guide.
