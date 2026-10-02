# Wheel-speed feedback and serial robustness

[한국어](../../ko/troubleshooting/supporting/02-wheel-speed-feedback.md) | [Troubleshooting index](../../troubleshooting.md)

**Priority:** Supporting · Reconstructed from existing documentation and code

## 1. Problem definition

A corrupted wheel-speed sample can change the common speed-correction term applied to both motors. Older notes describe implausible jumps and parsing concerns; the final sketch alone does not measure their frequency.

## 2. Candidate solutions

Compare raw serial responses with parsed speed; inspect wheel-direction signs and conversion to linear speed. Evaluate history-based rejection, median-style fallback, or smoothing against added delay. Distinguish archived experiments from the active controller.

## 3. Execution

Older documentation describes adaptive thresholds and median-style fallback in archived firmware. The final sketch drains currently available received bytes before and after `parseFloat()` reads, converts wheel feedback using direction signs and wheel radius, and bounds speed-integral/output values. Its recurrence uses `kAlpha_3=1`, which passes each new speed sample directly through: smoothing is effectively disabled. Draining available bytes is not the same as proving complete or valid response framing.

## 4. Reinterpreting the experience

A variable named 'filtered' does not establish that a filter is active. Stability depends on feedback validity and timing as well as correction limits. Historical outlier-handling work should not be presented as deployed protection without matching the active code.

## 5. Summary and verification

Outcome: receive-buffer draining and bounded speed correction are present; active speed smoothing and complete sample validation are not demonstrated. Review `BalanceController()` and archived firmware. Future checks should capture raw responses, parse duration/timeouts, sample age, and wheel-speed jumps under the same test sequence.

### Evidence

- [Physical controller](../../../firmware/physical_balance_controller/physical_balance_controller.ino)
- [Archived Arduino firmware](../../../archive/arduino_firmware)
