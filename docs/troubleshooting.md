# Troubleshooting: from observations to control and architecture changes

English | [한국어](ko/troubleshooting.md)

Unexpected wheel motion and falls did not have one cause. Separating RC input, motor drive, attitude reference, speed feedback, and ROS commands led to changes in both control and testing.

Each case connects **the problem → the investigation → the response → the changed interpretation → inspectable results**. Major cases concern safety and physical behavior; supporting cases explain the sensor, feedback, and software decisions behind them.

## Three cases to read first

| Case | Work performed | Inspectable evidence |
| --- | --- | --- |
| [Wheels moved without an intended command](troubleshooting/major/01-rc-pwm-spikes.md) | Isolate PWM inspection, investigate placement, change input handling | Tester output fields and final filter/deadband/engage logic |
| [Sudden motor acceleration](troubleshooting/major/02-odrive-runaway.md) | Separate RC/hall/direct-current paths, interpret firmware-change outcome | Four testers, state requests, current-request bounds |
| [From standing to driving](troubleshooting/major/03-balance-tuning.md) | Bench/tether tests, combine balance/speed/steering | Test photos, mixing equations, driving demos |

**For a short review:** read the opening outcome, inspect the observation table or photos, then follow the code excerpt and evidence links.

## Supporting cases

| Case | Decision demonstrated |
| --- | --- |
| [IMU zero versus upright reference](troubleshooting/supporting/01-imu-upright-reference.md) | Separate calibration from mechanical posture reference |
| [Speed-feedback handling](troubleshooting/supporting/02-wheel-speed-feedback.md) | Distinguish processing experiments from active code |
| [Navigation through balance control](troubleshooting/supporting/03-ros-command-path.md) | Separate high-level intent from low-level attitude control |

## Evidence scope

Tester output fields and excerpts establish implementation. Photos and short demos establish testing environments and visible behavior. Metal/foil observations and firmware-downgrade outcome come from existing project records; raw experiment logs are not published. Code-derived values are identified separately from measurements.

The repository reconstructs and organizes an earlier project. A tester's existence does not independently establish every historical test result. The cases focus on work and outcomes that can presently be inspected.

See the [control algorithm](../firmware/physical_balance_controller/control_algorithm.md) for complete equations, [results and limitations](results-and-limitations.md) for completion scope, and [Sim2Real](sim2real.md) for the simulation/hardware connection.
