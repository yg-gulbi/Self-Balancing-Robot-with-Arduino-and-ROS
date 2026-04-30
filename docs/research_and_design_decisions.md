# Research And Design Decisions

## Purpose

This page records the research that influenced the robot design without making the finished project sound bigger than it was.

The source material included recovered notes on CAN, depth cameras including the Orbbec Gemini 330, SLAM, FrSky receiver interference, and ROS autorun setup. I summarized those documents here instead of uploading raw office files.

## Decision Summary

| Topic | Status | What it informed | How I describe it in the repo |
| --- | --- | --- | --- |
| CAN protocol | Investigated, not used as the final implementation | CAN was studied as a robust vehicle or industrial communication protocol and as context for motor-controller communication options | The final firmware should be read as Arduino + ODrive control work, not as a completed CAN bus implementation |
| ODrive bring-up | Partially applied through physical testing | ODrive tooling, firmware setup, voltage checks, motor/hall-sensor connection, and controller modes informed the physical bring-up sequence | ODrive 3.6 motor control is supported by the firmware and hardware docs |
| Orbbec Gemini 330 depth camera | Confirmed as deployed sensor-head hardware; autonomous physical navigation use remains partial | Stereo depth, RGB/depth output, USB connection, IMU availability, and mounting constraints informed the upper sensor-head design and ROS integration experiments | The Gemini 330 can be listed as deployed hardware, but it does not mean I finished end-to-end physical autonomous navigation |
| ORB-SLAM2 | Investigated, not used as the final implementation | ORB-SLAM2 was reviewed for visual SLAM support across mono, stereo, and RGB-D inputs | The repository should not imply a completed ORB-SLAM2 physical robot deployment |
| RTAB-Map | Investigated as a strong RGB-D candidate | RTAB-Map was considered because of ROS support and RGB-D mapping suitability | It remains research/integration context unless backed by a reproducible launch and demo |
| SLAM and navigation concepts | Applied in simulation and launch experiments | Front-end/back-end SLAM concepts, loop closure, mapping, and navigation informed the ROS workflow | Simulation navigation is claimed; physical autonomous navigation is explicitly partial |
| FrSky radio receiver interference | Investigated as real-world troubleshooting | Wire length, poor joins, grounding, shielding, twisted signal-ground routing, ferrite beads, and filters were identified as noise factors in the RC control path | These are engineering lessons, not a guaranteed final build guide |
| ROS autorun | Investigated, not used as a final implementation | Shell script and systemd service patterns were reviewed for boot-time ROS startup | I am not presenting a production-ready autorun setup in this repository |

## Design Impact

- The project kept the physical control loop on Arduino with BNO055 feedback, ODrive motor commands, and a FrSky Taranis Q X7 -> X8R radio path because that stack gave the most solid real-world result.
- ROS work was kept as a simulation and integration layer, which keeps the repository honest about what was completed and what was still exploratory.
- The Orbbec Gemini 330 can be named as the sensor-head hardware used for physical integration experiments, while depth-based autonomous navigation still stays outside the repo's finished results.
- FrSky receiver-noise notes explain why hardware wiring and signal integrity mattered during testing, especially for RC drive control.

## Practical Lessons From Research

| Research area | Practical lesson | How it shaped the repo |
| --- | --- | --- |
| CAN and motor-control communication | Robust communication matters, but research alone does not mean a full bus architecture was finished | CAN is documented as context, while ODrive/Arduino firmware remains the main finished path |
| Orbbec Gemini 330 selection | Depth data can help mapping in low-texture environments, but physical integration still needs reproducible launch files and demos | The camera is shown as deployed hardware, while depth navigation stays on the research side |
| ORB-SLAM2 vs RTAB-Map | ORB-SLAM2 is attractive for visual SLAM, while RTAB-Map looked more practical for RGB-D ROS exploration | The repo describes both as evaluated options without overstating deployment |
| FrSky receiver interference | Long or poorly joined wires can behave like noise-sensitive signal paths | RC wiring lessons are captured as troubleshooting knowledge, not as a final wiring recipe |
| ROS autorun | Boot-time startup is useful only after the launch stack is stable | Autorun remains an investigated deployment topic, outside the completed core results |

## Documentation Rules

- Research documents support the design rationale; they do not automatically turn into finished deliverables.
- Raw office documents, chat exports, and datasheets are not required in the main GitHub tree.
- Any raw archive material should go to a GitHub Release or private archive first, then be summarized into public Markdown only after privacy and size review.
