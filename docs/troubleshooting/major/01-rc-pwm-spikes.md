# RC PWM spikes and unintended wheel twitch

[한국어](../../ko/troubleshooting/major/01-rc-pwm-spikes.md) | [Troubleshooting index](../../troubleshooting.md)

**Priority:** Major · Reconstructed from existing documentation and code

## 1. Problem definition

During physical bring-up, the wheels sometimes twitched without an intended command. Serial inspection also showed jumps in FrSky X8R PWM pulse widths. Separate an input fault from a motor fault before changing balance gains.

## 2. Candidate solutions

Consider three paths: receiver/antenna placement near conductive structures, PWM wiring and ground-reference quality, and software interpretation of noisy input. Inspect raw PWM with a receiver-only sketch; vary placement and connections; then compare filtering, neutral deadband, and engage persistence. These are reconstructed investigation options, not a dated experiment log.

## 3. Execution

The existing notes record receiver-only PWM inspection, repeated connection changes, sensitivity to metal proximity, and a foil experiment that worsened spikes. The controller implements throttle/steering alpha `0.4`, engage alpha `0.02`, neutral offsets, a `50 us` deadband, and engage persistence. Throttle deadband uses `1488 us`, while normalization uses `1491 us`; steering center is `1492 us`. Do not describe these as one perfectly unified calibration. Placement and close signal-ground routing remain mitigations; the records do not provide a controlled before/after noise-rate measurement.

## 4. Reinterpreting the experience

Receiver placement and wiring belong inside the control-system design. Filtering reduces the effect of noisy input; it does not prove whether RF detuning, induced noise, or ground-reference disturbance caused each spike. The metal/foil observations support a placement-related hypothesis, not a measured EMI root cause.

## 5. Summary and verification

Outcome: PWM filtering, deadband, offsets, and engage persistence are implemented. Limitation: no quantitative spike-reduction dataset is published. Review `PrintRcSignals()`, the filter constants, and `EngageMotors()` alongside the receiver tester. A future comparison should record raw/filtered PWM, receiver location, wiring condition, and unintended-command counts.

### Evidence

- [Receiver PWM tester](../../../firmware/testers/receiver_pwm_test/receiver_pwm_test.ino)
- [Physical controller](../../../firmware/physical_balance_controller/physical_balance_controller.ino)
- [Development process](../../../docs/development-process.md)
