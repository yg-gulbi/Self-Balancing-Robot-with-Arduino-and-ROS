# Troubleshooting Index

English | [한국어](ko/troubleshooting.md)

This index links to individual engineering issues. Each follows **problem definition → candidate solutions → execution → reinterpretation → summary and verification**. Priority reflects safety, core behavior, and investigation scope; supporting issues still affect stability.

## Major troubleshooting

| Document | Focus |
| --- | --- |
| [RC PWM spikes and unintended wheel twitch](troubleshooting/major/01-rc-pwm-spikes.md) | Noisy intent and unintended activation |
| [ODrive runaway and uncontrolled motor behavior](troubleshooting/major/02-odrive-runaway.md) | Motor-path isolation and downgrade outcome |
| [Physical balance tuning and fall-risk management](troubleshooting/major/03-balance-tuning.md) | Staged tests and balance/speed/steering control |

## Supporting troubleshooting

| Document | Focus |
| --- | --- |
| [IMU calibration and upright reference](troubleshooting/supporting/01-imu-upright-reference.md) | Calibration versus physical upright posture |
| [Wheel-speed feedback and serial robustness](troubleshooting/supporting/02-wheel-speed-feedback.md) | Outliers, parsing, and actual filter settings |
| [ROS navigation command path and balance authority](troubleshooting/supporting/03-ros-command-path.md) | Motion intent versus final control output |

## Evidence and reading conventions

- **Code-confirmed:** implementation and settings can be inspected; helper existence is distinguished from enabled behavior.
- **Previously reported:** the old documentation records an observation, but raw logs or quantitative comparisons may be absent.
- **Inference:** an explanation consistent with observations, not a proven cause.
- **Further verification:** each entry provides review steps; no physical tests were rerun during this documentation rewrite.

Candidate-solution lists reconstruct the options supported by existing material. They do not invent experiment order, measurements, or failure counts. Filtering and safety measures appear within the issues they address. See the [control algorithm](../firmware/physical_balance_controller/control_algorithm.md) for detailed control equations.

## Related documentation

- [Development process](development-process.md)
- [Results and limitations](results-and-limitations.md)
- [Sim2Real](sim2real.md)
