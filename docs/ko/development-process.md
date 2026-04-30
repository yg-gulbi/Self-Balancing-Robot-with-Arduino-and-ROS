# 개발 과정

[English](../development-process.md) | 한국어

## 목적

이 문서는 복구된 제작 과정 자료를 그대로 쌓아두는 대신, 읽기 쉬운 개발 이야기로 정리한 문서입니다. 원본 발표 자료, 개인 대화 기록, 큰 source file을 저장소에 그대로 넣기보다, 공개 가능한 핵심 흐름만 정리했습니다.

목표는 로봇이 concept, CAD assembly, wiring, bench test, tethered driving practice, simulation을 거쳐 최종 결과까지 어떻게 이어졌는지 보여주는 것입니다.

## Visual Build Story

### 1. Concept And Mechanical Assembly

복구된 CATIA 자료는 상단 sensor head와 내부 electronics bay를 가진 compact two-wheeled balancing robot이라는 의도된 물리 구조를 보여줍니다.

![CATIA internal assembly views](../../media/process/catia_internal_assembly_views.jpg)

CATIA assembly view는 하드웨어 이야기를 텍스트보다 쉽게 확인하게 해줍니다. 실제 wiring이 확정되기 전의 body volume, internal packaging direction, upper mast, front/rear layout을 보여줍니다.

### 2. Head, Body, And Packaging Iteration

| Head design | Body design |
| --- | --- |
| ![CATIA head design](../../media/process/catia_head_design.jpg) | ![CATIA body design](../../media/process/catia_body_design.jpg) |

이 자료들은 이 프로젝트가 단순한 software demo가 아니었음을 보여줍니다. Physical chassis, head enclosure, sensor placement, battery/compute space, service opening이 제작 과정에서 함께 고려되었습니다.

### 3. Wiring And Control Architecture

![Wiring Diagram](../../media/diagrams/wiring_diagram.png)

Wiring Diagram은 FrSky Taranis Q X7 / X8R radio path, Arduino Mega 2560, BNO055 IMU, ODrive 3.6, battery, voltage converters, onboard PC, Orbbec Gemini 330, auxiliary Arduino, relay module을 연결해서 보여줍니다. 현재 저장소에서 공개용 system-level wiring/control summary 역할을 합니다.

### 4. Wheel And Motor Bench Testing

![Wheel bench test](../../media/process/wheel_bench_test.jpg)

완성된 로봇 데모 전에 wheel, motor, control electronics를 open bench에서 테스트했습니다. 이 단계는 full body balancing 전에 ODrive bring-up, hall-sensor feedback, power wiring, controller communication을 확인하는 데 중요했습니다.

### 5. Tethered Driving Practice

![Tethered driving practice](../../media/process/tethered_driving_practice.jpg)

Tethered driving photo는 bench testing과 free driving 사이의 practical safety step을 보여줍니다. Balance와 drive parameter를 조정하는 동안 로봇을 overhead support line으로 지지한 상태에서 연습했습니다.

### 6. Simulation And Navigation

![Simulation navigation views](../../media/process/simulation_unreal_navigation_views.jpg)

Simulation screenshot은 ROS/Gazebo 쪽 parallel track을 보여줍니다. Simulated robot placement, map/navigation view, environment testing이 포함됩니다. 그래서 이 저장소는 simulation navigation을 완료된 software-side result로 다루고, physical autonomous navigation은 partial category로 분리합니다.

## Build Timeline

| Phase | Focus | What that stage meant | Outcome |
| --- | --- | --- | --- |
| Concept definition | Mobility, maneuverability, efficient space use를 가진 two-wheeled guide robot | Final presentation material에서 compact mobile guidance platform으로 project goal을 설명 | Project goal이 balancing robot + ROS navigation exploration으로 좁혀짐 |
| Subsystem planning | ODrive 3.6, Arduino Mega 2560, BNO055 IMU, FrSky Taranis Q X7 + X8R, Orbbec Gemini 330, power conversion | Research notes와 weekly report가 hardware control과 ROS navigation work를 분리 | Main workstream이 physical balance control과 ROS/Gazebo simulation으로 정리됨 |
| ODrive and motor bring-up | Ubuntu setup, ODrive tooling, voltage checks, firmware setup, motor/hall-sensor connection | Weekly report material에 full robot integration 전 staged motor-controller bring-up이 기록됨 | Balancing experiment 전 motor control path 검증 |
| Control integration | Arduino Mega 2560, BNO055 IMU, FrSky X8R receiver, ODrive command path | Firmware와 process notes가 IMU feedback, RC input, motor command integration을 보여줌 | Physical balancing과 RC driving이 가장 강한 real-world result가 됨 |
| Hardware assembly | Internal placement, wiring, battery pack, converter chain, relay/auxiliary area | Recovered wiring notes와 internal photo를 cleaner public diagrams/photos로 정리 | Hardware section이 raw photo와 Wiring Diagram을 중심으로 구성됨 |
| ROS simulation and navigation | Gazebo balancing robot, `/before_vel`, SLAM/navigation launch composition | ROS packages가 simulation control, mapping/navigation experiment, launch integration을 보여줌 | Simulation navigation은 completed로, physical autonomous navigation은 not completed로 분리 |
| Portfolio recovery | Demos, firmware, ROS packages, archived notes, lighter media | Raw project material을 그대로 복사하지 않고 요약 | Main result와 older experiment/research material이 분리됨 |

## Functional Workstreams

| Workstream | What it covered | Main files or media |
| --- | --- | --- |
| ODrive / motor | Motor controller setup, hall-sensor feedback, motor synchronization, current control experiments | `firmware/physical_balance_controller`, `firmware/testers/motor_current_test`, `firmware/testers/odrive_receiver_test`, `archive/arduino_firmware`, hardware photos |
| FrSky radio path | X8R PWM input, Taranis Q X7 manual commands, engage/throttle/steering interpretation | `physical_balance_controller.ino`, `receiver_pwm_test.ino`, `rc_to_ros_cmd_vel_bridge.ino`, receiver troubleshooting notes |
| IMU / balancing | BNO055 angle/gyro feedback, balance-loop tuning, safety-constrained parameter testing | `physical_balance_controller.ino`, hallway and obstacle-course demos |
| ROS / navigation | Gazebo balancing simulation, `/before_vel` command separation, SLAM/navigation launch files | `ros_ws/src/balance_robot_control`, `ros_ws/src/navigation`, `ros_ws/src/balance_robot_workflows` |
| Hardware assembly | Chassis packaging, internal electronics bay, battery and DC-DC power chain | `media/hardware`, `media/diagrams/wiring_diagram.png` |
| Research and documentation | CAN, Orbbec Gemini 330, ORB-SLAM2, RTAB-Map, FrSky receiver noise, ROS autorun investigation | 이 문서, `docs/ko/troubleshooting.md`, `docs/ko/results-and-limitations.md` |

## Research And Design Decisions

복구된 research notes는 프로젝트 방향에 영향을 주었지만, 그것 자체가 별도의 완성 deliverable은 아닙니다. 저장소가 과하게 분산되지 않도록 공개에 필요한 decision만 여기에 요약했습니다.

| Topic | What it influenced | Final repo framing |
| --- | --- | --- |
| CAN protocol | Vehicle-style 또는 industrial motor-control system에서의 communication option | 배경 지식으로만 다룹니다. 최종 구현은 Arduino + ODrive이며 completed CAN bus system이 아닙니다 |
| ODrive bring-up | Voltage checks, firmware setup, hall-sensor connection, controller modes, current-control testing | Physical balancing result를 뒷받침하는 실제 subsystem bring-up으로 다룹니다 |
| Orbbec Gemini 330 | Sensor-head direction, depth-camera placement, physical ROS integration experiments | Deployed hardware로 기록하지만, physical autonomous depth navigation은 partial로 둡니다 |
| ORB-SLAM2 and RTAB-Map | Mapping/navigation workflow를 위한 visual/RGB-D SLAM tradeoff research | Research context와 simulation/integration background로 남기고, finished physical deployment로 과장하지 않습니다 |
| FrSky receiver interference | Wire length, grounding, shielding, twisted signal-ground routing, filtering | 실제 RC control reliability에 직접 영향을 준 troubleshooting story로 통합했습니다 |
| ROS autorun | Script/service 기반 boot-time launch pattern | 조사한 항목으로만 기록하고 production-ready robot startup system으로 제시하지 않습니다 |

## Component Bring-Up Process

복구된 process material은 각 주요 부품을 full robot의 일부로 다루기 전에 별도로 bring-up했다는 점을 보여줍니다. 최종 데모 뒤의 가장 중요한 engineering story는 로봇이 각 component를 testable role로 줄여가면서 안정화되었다는 것입니다.

| Component | Bring-up goal | Trial-and-error process | Stabilized use in the project |
| --- | --- | --- | --- |
| ODrive 3.6 | Balancing 전 두 wheel motor를 안정적으로 구동 | Ubuntu/ODrive tooling setup, input voltage check, motor phase/hall sensor connection, controller mode test, simple spin test에서 current command로 이동 | Arduino firmware가 command하는 current-control motor driver로 사용 |
| Wheel motors + hall sensors | Balancing과 drive correction에 필요한 wheel motion read | Motor speed feedback 비교, abnormal reading filtering, hall-state transition count, speed-related feedback tuning | Physical balance control 중 wheel speed feedback과 motor synchronization에 사용 |
| BNO055 IMU | Balance loop를 위한 body angle/gyro feedback 제공 | Sensor startup 확인, calibration data 처리, EEPROM에 offset 저장, angle/gyro reading을 core balancing signal로 사용 | `physical_balance_controller.ino`의 main physical attitude sensor |
| FrSky X8R + Taranis Q X7 | Manual remote input을 safe steering/throttle/engage command로 변환 | Interrupt로 PWM pulse width 측정, noisy channel value filtering, neutral threshold 추가, accidental activation 방지를 위한 engage persistence | Balancing 중 manual driving과 RC-to-ROS bridge testing에 사용 |
| Arduino Mega 2560 | Low-level balance loop를 ROS와 독립적으로 deterministic하게 유지 | PWM input, IMU reading, ODrive serial communication, safety check, current-command output을 하나의 firmware loop에 통합 | 실제 로봇 동작의 main physical controller |
| Power chain | Motor power, onboard compute, low-voltage auxiliary를 하나의 robot package에서 공급 | Recovered wiring notes가 36V main bus, 36V to 19V stage, 19V to 5V stage를 보여줌 | Clean power/IO diagram과 internal hardware photo로 문서화 |
| Orbbec Gemini 330 | SLAM/navigation을 위한 perception hardware 탐색 | Physical autonomous navigation이 완전히 끝나기 전에 depth-camera capability와 SLAM approach를 비교 | Deployed sensor-head hardware와 research context로 유지 |
| ROS/Gazebo stack | Hardware risk 없이 balancing/navigation idea test | Simulation launch files 구성, `/before_vel`로 navigation command input 분리, Gazebo에서 control behavior tuning | Main simulation and navigation track으로 사용 |

## Trial-And-Error Highlights

| Challenge | Iteration path | Lesson captured in the repository |
| --- | --- | --- |
| Real balancing은 작은 tuning change에도 민감했음 | Subsystem check부터 시작해 IMU angle, gyro rate, wheel speed feedback, ODrive current command를 safety limit 아래에서 결합 | Physical balancing은 흩어진 raw note보다 final controller와 demo를 함께 볼 때 가장 잘 이해됩니다 |
| RC input이 neutral 주변에서 튀거나 drift할 수 있었음 | PWM filtering, neutral threshold, engage persistence를 추가 | Manual control은 단순 wiring이 아니라 signal-processing problem으로 다뤄야 했습니다 |
| Motor feedback을 그대로 믿으면 unstable behavior가 생길 수 있었음 | Encoder speed history와 filtering으로 abnormal reading 영향을 줄임 | Feedback quality는 controller equation만큼 중요했습니다 |
| IMU calibration이 repeatability에 영향을 줌 | BNO055 calibration offset 저장/로드, calibration status serial output | Sensor calibration은 operating process의 일부가 되었습니다 |
| Hardware packaging은 해석 위험이 있었음 | Internal photo는 실제 layout을 보여주지만 직접 label을 붙이면 부품을 과하게 특정할 수 있음 | Public docs는 uncertain photo annotation 대신 raw photo + clean diagram을 사용합니다 |
| SLAM/navigation ambition은 finished physical result보다 더 넓었음 | Visual/depth SLAM과 RTAB-Map/ORB-SLAM2를 research하면서 simulation navigation이 먼저 성숙 | Completed simulation work와 partial physical integration을 분리합니다 |

## Problems And Resolutions

| Problem | What was learned | Portfolio-safe result |
| --- | --- | --- |
| 실제 로봇 balance tuning은 hall-sensor/IMU feedback에 민감했음 | Physical testing에는 staged safety setup과 iterative parameter adjustment가 필요 | Real balancing은 demo media와 firmware로 보여줌 |
| FrSky receiver signal은 wire length와 noise concern에 취약했음 | Shorter wiring, better connections, shielding/twisted signal-ground routing, filtering이 mitigation으로 확인됨 | Troubleshooting을 final wiring guarantee가 아니라 engineering process로 문서화 |
| Depth-camera와 SLAM option에는 integration uncertainty가 있었음 | Orbbec Gemini 330, ORB-SLAM2, RTAB-Map을 physical navigation 완료 전 비교 | Simulation/integration experiment를 physical balancing result와 분리 |
| Raw project files에는 유용한 자료와 private/noisy material이 섞여 있었음 | Public docs는 technical process만 요약하고 chat logs/raw decks는 제외 | GitHub 저장소를 깔끔하게 유지 |

## Final Summary

- 실제 로봇 balancing과 RC driving은 완료되었고, firmware와 demo media로 뒷받침됩니다.
- ROS/Gazebo balancing, SLAM-related launch composition, simulation navigation은 software-side work로 완료되었습니다.
- Real-world ROS SLAM/navigation integration은 experiment로 존재하지만, physical robot의 full autonomous navigation으로 제시하지 않습니다.
- Raw process artifact는 primary public deliverable이 아니라 이 요약 문서들의 source material로 다룹니다.
