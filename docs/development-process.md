# Development Process

## Purpose

This page organizes the recovered build-process material into a readable story instead of dumping raw presentation decks, private chat logs, or large source files into the repository.

The goal is to show how the robot moved from concept, CAD assembly, wiring, bench tests, tethered driving practice, and simulation into the final project results.

## Visual Build Story

### 1. Concept And Mechanical Assembly

The recovered CATIA material shows the intended physical form: a compact two-wheeled balancing robot with an upper sensor head and an internal electronics bay.

![CATIA internal assembly views](../media/process/catia_internal_assembly_views.jpg)

The CATIA assembly views make the hardware story easier to inspect than text alone. They show the body volume, internal packaging direction, upper mast, and rear/front layout before the physical wiring was finalized.

### 2. Head, Body, And Packaging Iteration

| Head design | Body design |
| --- | --- |
| ![CATIA head design](../media/process/catia_head_design.jpg) | ![CATIA body design](../media/process/catia_body_design.jpg) |

These slides show that the robot was not only a software demo. The physical chassis, head enclosure, sensor placement, battery/compute space, and service opening were considered as part of the build.

### 3. Wiring And Control Architecture

![Wiring Diagram](../media/diagrams/wiring_diagram.png)

The Wiring Diagram connects the FrSky Taranis Q X7 / X8R radio path, Arduino Mega 2560, BNO055 IMU, ODrive 3.6, battery, voltage converters, onboard PC, Orbbec Gemini 330, auxiliary Arduino, and relay module. It is the repository's current public system-level summary of the robot wiring and control architecture.

### 4. Wheel And Motor Bench Testing

![Wheel bench test](../media/process/wheel_bench_test.jpg)

Before the completed robot demos, the wheel/motor/control electronics were tested on an open bench. This stage was important for checking ODrive bring-up, hall-sensor feedback, power wiring, and controller communication before balancing the full body.

### 5. Tethered Driving Practice

![Tethered driving practice](../media/process/tethered_driving_practice.jpg)

The tethered driving photos show the practical safety step between bench testing and free driving. The robot was practiced with an overhead support line while balance and drive parameters were adjusted.

### 6. Simulation And Navigation

![Simulation navigation views](../media/process/simulation_unreal_navigation_views.jpg)

The simulation screenshots show the parallel ROS/Gazebo track: simulated robot placement, map and navigation views, and environment testing. This is why the repository treats simulation navigation as a finished software-side result, while physical autonomous navigation stays in the partial category.

## Build Timeline

| Phase | Focus | What that stage meant | Outcome |
| --- | --- | --- | --- |
| Concept definition | Two-wheeled guide robot with mobility, maneuverability, and efficient space use | Final presentation material framed the robot as a compact mobile guidance platform | Project goal narrowed to a balancing robot plus ROS navigation exploration |
| Subsystem planning | ODrive 3.6, Arduino Mega 2560, BNO055 IMU, FrSky Taranis Q X7 + X8R, Orbbec Gemini 330, and power conversion | Research notes and weekly reports separated hardware control from ROS navigation work | Main workstreams became physical balance control and ROS/Gazebo simulation |
| ODrive and motor bring-up | Ubuntu setup, ODrive tooling, voltage checks, firmware setup, motor and hall-sensor connection | Weekly report material records staged motor-controller bring-up before full robot integration | Motor control path was validated before balancing experiments |
| Control integration | Arduino Mega 2560, BNO055 IMU, FrSky X8R receiver, and ODrive command path | Firmware and process notes show IMU feedback, RC input, and motor command integration | Physical balancing and RC driving became the strongest real-world result |
| Hardware assembly | Internal placement, wiring, battery pack, converter chain, relay/auxiliary area | Recovered wiring notes and the internal photo were turned into cleaner public diagrams and photos | The hardware section now uses the raw photo plus the Wiring Diagram |
| ROS simulation and navigation | Gazebo balancing robot, `/before_vel`, SLAM/navigation launch composition | ROS packages show simulation control, mapping and navigation experiments, and launch integration | Simulation navigation is treated as completed; physical autonomous navigation is not |
| Portfolio recovery | Demos, firmware, ROS packages, archived notes, and lighter media | Raw project material was summarized instead of copied wholesale | The repository now separates the main results from older experiments and research material |

## Functional Workstreams

| Workstream | What it covered | Main files or media |
| --- | --- | --- |
| ODrive / motor | Motor controller setup, hall-sensor feedback, motor synchronization, current control experiments | `firmware/physical_balance_controller`, `firmware/testers/motor_current_test`, `firmware/testers/odrive_receiver_test`, `archive/arduino_firmware`, hardware photos |
| FrSky radio path | X8R PWM input, Taranis Q X7 manual commands, and engage/throttle/steering interpretation | `physical_balance_controller.ino`, `receiver_pwm_test.ino`, `rc_to_ros_cmd_vel_bridge.ino`, receiver troubleshooting notes |
| IMU / balancing | BNO055 angle and gyro feedback, balance-loop tuning, safety-constrained parameter testing | `physical_balance_controller.ino`, hallway and obstacle-course demos |
| ROS / navigation | Gazebo balancing simulation, `/before_vel` command separation, SLAM/navigation launch files | `ros_ws/src/balance_robot_control`, `ros_ws/src/navigation`, `ros_ws/src/balance_robot_workflows` |
| Hardware assembly | Chassis packaging, internal electronics bay, battery and DC-DC power chain | `media/hardware`, `media/diagrams/wiring_diagram.png` |
| Research and documentation | CAN, Orbbec Gemini 330, ORB-SLAM2, RTAB-Map, FrSky receiver noise, and ROS autorun investigation | This page, `docs/troubleshooting.md`, and `docs/results-and-limitations.md` |

## Research And Design Decisions

The recovered research notes helped shape the project, but they are not separate finished deliverables. The important public-facing decisions are summarized here so the repository stays compact.

| Topic | What it influenced | Final repo framing |
| --- | --- | --- |
| CAN protocol | Communication options for vehicle-style or industrial motor-control systems | Useful background only; the final implementation is Arduino + ODrive, not a completed CAN bus system |
| ODrive bring-up | Voltage checks, firmware setup, hall-sensor connection, controller modes, and current-control testing | Treated as real subsystem bring-up that supports the physical balancing result |
| Orbbec Gemini 330 | Sensor-head direction, depth-camera placement, and physical ROS integration experiments | Listed as deployed hardware, while physical autonomous depth navigation remains partial |
| ORB-SLAM2 and RTAB-Map | Visual/RGB-D SLAM tradeoff research for mapping and navigation workflows | Kept as research context and simulation/integration background, not overstated as a finished physical deployment |
| FrSky receiver interference | Wire length, grounding, shielding, twisted signal-ground routing, and filtering | Folded into the troubleshooting story because it directly affected real RC control reliability |
| ROS autorun | Boot-time launch patterns using scripts or services | Documented only as investigated; not presented as a production-ready robot startup system |

## Component Bring-Up Process

The recovered process material shows that each major part was brought up separately before being treated as part of the full robot. This is the most important engineering story behind the final demos: the robot became stable only after the team reduced each component to a testable role.

| Component | Bring-up goal | Trial-and-error process | Stabilized use in the project |
| --- | --- | --- | --- |
| ODrive 3.6 | Drive both wheel motors reliably before balancing | Set up Ubuntu/ODrive tooling, checked input voltage, connected motor phases and hall sensors, tested controller modes, then moved from simple spin tests toward current commands; the repository now keeps a focused motor-current test sketch for that bring-up path | Used as the current-control motor driver commanded by Arduino firmware |
| Wheel motors + hall sensors | Read wheel motion well enough for balancing and drive correction | Compared motor speed feedback, filtered abnormal readings, counted hall-state transitions, and tuned speed-related feedback so the two wheels could respond together | Used for wheel speed feedback and motor synchronization during physical balance control |
| BNO055 IMU | Provide body angle and gyro feedback for the balance loop | Verified sensor startup, handled calibration data, stored offsets in EEPROM, and used angle/gyro readings as the core balancing signal | Used as the main physical attitude sensor in `physical_balance_controller.ino` |
| FrSky X8R + Taranis Q X7 | Convert manual remote input into safe steering, throttle, and engage commands | Measured PWM pulse width with interrupts, filtered noisy channel values, added thresholds around neutral, and used engage persistence to avoid accidental motor activation | Used for manual driving while balancing and for RC-to-ROS bridge testing |
| Arduino Mega 2560 | Keep the low-level balance loop deterministic and independent from ROS | Integrated PWM input, IMU readings, ODrive serial communication, safety checks, and current-command output in one firmware loop | Became the main physical controller for the completed real robot behavior |
| Power chain | Feed motor power, onboard compute, and low-voltage auxiliaries from one robot package | Recovered wiring notes show a 36V main bus, a 36V to 19V conversion stage, and a 19V to 5V stage for lower-voltage electronics | Documented through the clean power/IO diagram and internal hardware photo |
| Orbbec Gemini 330 | Explore perception hardware for SLAM/navigation | Compared depth-camera capability and SLAM approaches before the physical autonomous navigation path was fully completed | Kept as deployed sensor-head hardware and research context, not as a finished physical navigation result |
| ROS/Gazebo stack | Test balancing/navigation ideas without risking hardware | Built simulation launch files, separated navigation command input through `/before_vel`, and tuned control behavior in Gazebo | Used as the main simulation and navigation track |

## Trial-And-Error Highlights

| Challenge | Iteration path | Lesson captured in the repository |
| --- | --- | --- |
| Real balancing was sensitive to small tuning changes | Started with subsystem checks, then combined IMU angle, gyro rate, wheel speed feedback, and ODrive current commands under safety limits | Physical balancing makes the most sense when you look at the final controller and demos, not just scattered raw notes |
| RC input could jump or drift around neutral | PWM values were filtered, neutral thresholds were added, and engage logic required persistence before motor activation | Manual control was treated as a signal-processing problem, not just a wiring task |
| Motor feedback could produce unstable behavior if trusted blindly | Encoder speed history and filtering were used to reduce the effect of abnormal readings | Feedback quality mattered as much as the controller equation |
| IMU calibration affected repeatability | BNO055 calibration offsets were saved and loaded, and calibration status was exposed through serial output | Sensor calibration became part of the operating process |
| Hardware packaging created interpretation risk | Internal photos showed the real layout, but exact labels could be misleading if drawn directly on the image | Public docs now use raw photos plus a clean diagram instead of uncertain photo annotations |
| SLAM/navigation ambition went further than the finished physical result | Visual/depth SLAM and RTAB-Map/ORB-SLAM2 were researched while simulation navigation matured | The repository separates completed simulation work from partial physical integration |

## Problems And Resolutions

| Problem | What was learned | Portfolio-safe result |
| --- | --- | --- |
| Balance tuning on the real robot was sensitive to hall-sensor and IMU feedback | Physical testing needed staged safety setup and iterative parameter adjustment | Real balancing is shown through the physical demo media and firmware |
| FrSky receiver signals were vulnerable to wiring length and noise concerns | Shorter wiring, better connections, shielding/twisted signal-ground routing, and filtering were identified as mitigations | Troubleshooting is documented as engineering process, not as a final wiring guarantee |
| Depth-camera and SLAM options had integration uncertainty | Orbbec Gemini 330, ORB-SLAM2, and RTAB-Map were compared before final physical navigation was complete | Simulation and integration experiments are documented separately from completed physical balancing |
| Raw project files mixed useful material with private or noisy material | Public docs summarize the technical process and omit chat logs and raw decks | GitHub stays cleaner and avoids publishing unnecessary personal material |

## Final Summary

- Real robot balancing and RC driving were completed and are backed by firmware plus demo media.
- ROS/Gazebo balancing, SLAM-related launch composition, and simulation navigation were completed as software-side work.
- Real-world ROS SLAM/navigation integration existed as experiments, but full autonomous navigation on the physical robot is not presented as finished.
- Raw process artifacts are treated as source material for these summaries, not as primary public deliverables.
