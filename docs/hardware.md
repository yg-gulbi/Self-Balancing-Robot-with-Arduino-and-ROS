# Hardware

English | [한국어](ko/hardware.md)

This page is the single hardware reference for the physical self-balancing robot. It combines the previous BOM, power/IO notes, layout interpretation, and gallery material so people do not have to jump across several small hardware documents.

<table>
  <tr>
    <td width="38%">
      <img src="../media/hardware/robot_open_front.png" alt="Internal hardware photo" width="100%">
    </td>
    <td width="62%">
      <img src="../media/diagrams/wiring_diagram.png" alt="Wiring Diagram" width="100%">
    </td>
  </tr>
  <tr>
    <td valign="top">
      <strong>Physical layout</strong><br>
      Real chassis, sensor head, electronics bay, battery area, compute enclosure, and wheel base.
    </td>
    <td valign="top">
      <strong>System wiring summary</strong><br>
      Power chain, Arduino, BNO055, FrSky receiver, ODrive, motors, onboard PC, Gemini 330, auxiliary Arduino, and relay path.
    </td>
  </tr>
</table>

## Confirmed Components

| Component | Identification | Role |
| --- | --- | --- |
| Main controller | Arduino Mega 2560 | Runs the physical balance, RC input, safety, and ODrive command loop |
| IMU | BNO055 | Body angle and gyro feedback for balancing |
| Motor controller | ODrive 3.6 | Current-based dual-motor control |
| Depth camera | Orbbec Gemini 330 | Upper sensor-head perception hardware for ROS integration experiments |
| RC transmitter | FrSky 2.4GHz Taranis Q X7 | Handheld throttle, steering, and engage control |
| RC receiver | FrSky X8R | PWM command input into Arduino |

## Best-Effort Hardware Identification

Some exact part numbers were not retained, so these are described conservatively from firmware, wiring diagrams, photos, and process material.

| Subsystem | Best-effort identification | Why I think so |
| --- | --- | --- |
| Wheels and motors | Dual 36V hall-sensor BLDC hub motors, likely hoverboard-style, roughly 165 mm wheel diameter | Firmware uses hall feedback and about `0.0825 m` wheel radius; photos match hub-wheel assemblies |
| Battery | 36V battery pack, likely lithium-ion | Wiring diagram and internal photo both show a 36V system bus |
| Power conversion | 36V -> 19V and 19V -> 5V DC-DC converters | Wiring diagram shows compute and low-voltage control rails |
| Onboard compute | 19V mini PC, likely x86/NUC-class | Wiring diagram labels a PC rail; internal photo shows a vented compute enclosure |
| Auxiliary controller | Arduino Mini/Nano-class board | Wiring diagram labels a mini Arduino on the 5V side |
| Relay path | 5V relay or switching module | Wiring diagram shows an auxiliary relay/power-relay block |
| Chassis | 3D-printed body, mast, and sensor-head mount on a metal base plate | CAD/process images and physical photos support a purpose-built enclosure |

## Power And Signal Flow

```text
36V Battery
  -> ODrive 3.6 -> dual wheel motors
  -> 36V->19V converter -> onboard PC
  -> 19V->5V converter -> auxiliary Arduino / relay module

FrSky Taranis Q X7 -> FrSky X8R receiver -> Arduino Mega 2560
BNO055 IMU -> Arduino Mega 2560
Arduino Mega 2560 -> ODrive 3.6 command path
Orbbec Gemini 330 -> onboard PC
```

The physical control path is intentionally simple: `FrSky receiver + BNO055 -> Arduino -> ODrive -> motors`. ROS and perception hardware sit above that layer and are not required for the real-time balancing loop.

## Layout Interpretation

The public photo is left mostly unannotated because earlier direct callouts risked over-identifying parts. The reliable visible zones are:

- upper Orbbec Gemini 330 sensor-head area
- left-side auxiliary board area
- central compute and wiring bay
- right-side 36V battery pack
- open front service area exposing the internal electronics

## Historical Camera Naming Note

Some simulation-side URDF and launch files still reference `RealSense D435` assets or `d435` topic names. Those belong to historical simulation or placeholder RGB-D workflows. The deployed physical camera documented here is the Orbbec Gemini 330.

## Limits Of This Page

- The Wiring Diagram is a high-level project diagram, not a manufacturing schematic.
- Wire colors, connector pinouts, regulator part numbers, and exact battery chemistry should not be inferred beyond what is visible in the material.
- This page is written for portfolio review and system understanding, not as a complete rebuild guide.

## Supporting Images

| Asset | Why it matters |
| --- | --- |
| [Wiring Diagram](../media/diagrams/wiring_diagram.png) | Main hardware, power, and wiring overview |
| [Open-front robot photo](../media/hardware/robot_open_front.png) | Real electronics packaging and sensor-head layout |
| [Wheel bench test](../media/process/wheel_bench_test.jpg) | Motor, wheel, and controller bring-up before full-body balancing |
| [Tethered driving practice](../media/process/tethered_driving_practice.jpg) | Safety-supported physical tuning stage |
