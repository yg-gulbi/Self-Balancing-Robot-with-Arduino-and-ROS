# Research And Design Decisions

## Purpose

This page records the project research that influenced the robot design while keeping final implementation claims conservative.

The source material included recovered notes on CAN, depth cameras, SLAM, RC receiver interference, and ROS autorun setup. Those documents are summarized here instead of being published as raw office files.

## Decision Summary

| Topic | Status | What it informed | Public claim boundary |
| --- | --- | --- | --- |
| CAN protocol | Investigated, not claimed as final implementation | CAN was studied as a robust vehicle/industrial communication protocol and as context for motor-controller communication options | The final public firmware evidence should be read as Arduino + ODrive control work, not as a completed CAN bus implementation |
| ODrive bring-up | Partially applied through physical testing | ODrive tooling, firmware setup, voltage checks, motor/hall-sensor connection, and controller modes informed the physical bring-up sequence | ODrive 3.6 motor-control evidence is supported by firmware and hardware docs |
| Gemini depth camera | Investigated, not claimed as final physical navigation | Stereo depth, RGB/depth output, USB connection, IMU availability, and mounting constraints informed the upper sensor-head design | The camera/depth sensor is documented as a sensor head unless the exact deployed model is proven in public evidence |
| ORB-SLAM2 | Investigated, not claimed as final implementation | ORB-SLAM2 was reviewed for visual SLAM support across mono, stereo, and RGB-D inputs | The repository should not imply a completed ORB-SLAM2 physical robot deployment |
| RTAB-Map | Investigated as a strong RGB-D candidate | RTAB-Map was considered because of ROS support and RGB-D mapping suitability | It remains research/integration context unless backed by a reproducible launch and demo |
| SLAM and navigation concepts | Applied in simulation and launch experiments | Front-end/back-end SLAM concepts, loop closure, mapping, and navigation informed the ROS workflow | Simulation navigation is claimed; physical autonomous navigation is explicitly partial |
| RC receiver interference | Investigated as real-world troubleshooting | Wire length, poor joins, grounding, shielding, twisted signal-ground routing, ferrite beads, and filters were identified as noise factors | These are engineering lessons, not a guaranteed final build guide |
| ROS autorun | Investigated, not claimed as final implementation | Shell script and systemd service patterns were reviewed for boot-time ROS startup | No production-ready autorun setup is claimed in the public repository |

## Design Impact

- The project kept the physical control loop on Arduino with BNO055 feedback and ODrive motor commands because that path produced the strongest real-world evidence.
- ROS work was kept as a simulation and integration layer, which makes the repository honest about what was completed versus what was explored.
- Depth-camera and SLAM research are useful context for the sensor-head design, but they stay outside the main completion claims unless directly supported by code and demos.
- Receiver-noise notes explain why hardware wiring and signal integrity mattered during testing, especially for RC drive control.

## Practical Lessons From Research

| Research area | Practical lesson | How it shaped the public repo |
| --- | --- | --- |
| CAN and motor-control communication | Robust communication matters, but research alone is not evidence of a completed bus architecture | CAN is documented as context, while ODrive/Arduino firmware remains the verified claim |
| Depth-camera selection | Depth data can help mapping in low-texture environments, but physical integration still needs reproducible launch files and demos | The sensor head is shown as hardware evidence, while depth navigation stays a research boundary |
| ORB-SLAM2 vs RTAB-Map | ORB-SLAM2 is attractive for visual SLAM, while RTAB-Map looked more practical for RGB-D ROS exploration | The repo describes both as evaluated options without overstating deployment |
| Receiver interference | Long or poorly joined wires can behave like noise-sensitive signal paths | RC wiring lessons are captured as troubleshooting knowledge, not as a final wiring recipe |
| ROS autorun | Boot-time startup is useful only after the launch stack is stable | Autorun remains an investigated deployment topic, outside the completed core results |

## Public Documentation Rules

- Research documents support design rationale; they do not expand the list of completed deliverables by themselves.
- Raw office documents, chat exports, and datasheets are not required in the main GitHub tree.
- Any future raw evidence should go to a GitHub Release or private archive first, then be summarized into public Markdown only after privacy and size review.
