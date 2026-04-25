# Hardware BOM

## Confirmed Components

| Component | Role | Evidence |
| --- | --- | --- |
| Arduino Mega 2560 | Main low-level controller for the physical balancing robot | `Chimp4.ino` header comments and Arduino firmware archive |
| BNO055 IMU | Body angle and gyro feedback for balancing | `Chimp4.ino`, `sketch_jan20a.ino` |
| ODrive 3.6 | Current-based motor control | `Chimp4.ino`, `chimp_32.ino` |
| RC receiver | Manual drive and engage input via PWM | `Chimp4.ino`, `sketch_jan20a.ino` |
| Battery pack | Main power source | visible in internal photo, hand-drawn wiring sketch |
| DC-DC conversion chain | Power conversion for onboard electronics | hand-drawn wiring sketch |
| Onboard compute unit | Higher-level onboard computing visible in the chassis | internal photo, wiring sketch `PC` block |
| Relay / auxiliary board area | Auxiliary switching or interface zone on the left side of the chassis | internal photo, wiring sketch `릴레이 모듈` |
| Depth camera / sensor head | Perception hardware mounted on the upper mast | internal photo, CAD/mesh naming, URDF camera link |
| Robot body / neck / head structure | 3D body structure and mounted upper section | CAD/mesh files in `real_br_ws_intel/src/robot_description/mesh` |
| Wheel pair | Two-wheel balancing base | physical photos, URDF, firmware |

## Visual Component Evidence

- [CATIA concept overview](../media/process/catia_concept_overview.jpg): overall body, wheel base, mast, and sensor-head concept.
- [CATIA internal assembly views](../media/process/catia_internal_assembly_views.jpg): internal packaging and chassis volume evidence.
- [Source wiring block diagram](../media/process/source_wiring_block_diagram.jpg): recovered wiring/control architecture source used for the public diagram.
- [Wheel bench test](../media/process/wheel_bench_test.jpg): motor, wheel, power, and controller bench-test evidence before full robot practice.
- [Tethered driving practice](../media/process/tethered_driving_practice.jpg): safety-supported driving practice during physical tuning.

## Structure Evidence

Recovered mesh names indicate the project included modeled physical structure files for:

- robot body
- robot neck
- head parts
- a camera-case-like upper enclosure

This supports the hardware narrative that the project included both working electronics and a purpose-built physical chassis.

## Deliberate Omissions

The following are intentionally not over-specified until they are separately verified:

- exact battery chemistry and capacity
- exact motor model
- exact PC model
- final released camera model name
