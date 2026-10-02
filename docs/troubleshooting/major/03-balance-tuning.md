# Physical balance tuning and fall-risk management

[한국어](../../ko/troubleshooting/major/03-balance-tuning.md) | [Troubleshooting index](../../troubleshooting.md)

**Priority:** Major · Reconstructed from existing documentation and code

## 1. Problem definition

Small changes in gains, posture reference, wheel feedback, and actuator response could change whether the robot recovered or fell. Full-robot tests mixed several causes, making gain-only tuning difficult to interpret.

## 2. Candidate solutions

Separate subsystem bring-up from balance tuning. Use wheel/motor bench tests and tethered practice before free driving. Combine body-angle/angular-rate feedback with speed and steering correction, then constrain actuation and integral accumulation. The documents support this staged approach, but not a complete chronological gain-search history.

## 3. Execution

The final balance term is `(K_theta/100)*theta - (K_theta_dot/100)*theta_dot + (Ki/100)*integral_error`. Settings are `K_theta=24`, `K_theta_dot=1.7`, `Ki=0`; the angle-integral path exists but contributes zero at this setting. Speed correction uses `Kp=1.2`, `Ki=0.1`, `Kd=7`, with bounded accumulation and output. Its difference/sum terms are per update, without explicit `dt` normalization. Steering is mixed with opposite signs into the two current requests. The firmware uses a `30 deg` tilt threshold and `+/-8 A` current clamp; process photos show bench and tethered testing. The notes place bracket contact near `28 deg`, so the cutoff must not be described as guaranteed pre-contact protection.

## 4. Reinterpreting the experience

The tuning problem included signal trust, actuator bounds, and valid operating states as well as gains. The deployed controller is a layered balance/speed/steering implementation. Its equations and settings do not establish an optimally derived LQR controller or a fixed-rate continuous-time PID implementation.

## 5. Summary and verification

Project-level reported results are about one hour of balance and 10 m hallway/obstacle-course driving, supported by result documentation and demo media. They do not isolate the contribution of each gain or safeguard. Review the controller, control-algorithm document, and tethered/demo media. For repeatable tuning, log pitch, wheel speed, loop duration, commanded current, gain set, and fall/stop criteria.

### Evidence

- [Physical controller](../../../firmware/physical_balance_controller/physical_balance_controller.ino)
- [Control algorithm](../../../firmware/physical_balance_controller/control_algorithm.md)
- [Build process](../../../docs/development-process.md)
- [Results and limitations](../../../docs/results-and-limitations.md)
- [Tethered practice](../../../media/process/tethered_driving_practice.jpg)
- [Hallway demo](../../../media/hero/physical_balance_hallway.gif)
