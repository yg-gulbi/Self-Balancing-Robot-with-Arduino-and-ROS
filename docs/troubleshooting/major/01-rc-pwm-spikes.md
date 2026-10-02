# The wheels moved without an intended command: inspect the RC input first

[한국어](../../ko/troubleshooting/major/01-rc-pwm-spikes.md) | [All troubleshooting](../../troubleshooting.md)

> I separated FrSky X8R PWM inspection from motor control, investigated placement-dependent behavior, and changed how input was interpreted. Channel-specific filters, neutral deadband, and engage persistence remain in the final code.

## Wheel motion alone did not identify a gain problem

During physical bring-up, the wheels sometimes twitched without an intended command. The visible symptom was motor motion, but that did not establish a fault in the balance gains. A bad receiver command and a motor path reacting incorrectly to a valid command required different checks.

I moved the observation point from wheel motion to **the PWM pulse width entering Arduino**. If the input itself jumped, changing balance gains would not address the command interpretation problem.

## What the receiver-only test exposed

The [receiver tester](../../../firmware/testers/receiver_pwm_test/receiver_pwm_test.ino) inspects throttle, steering, and engage without driving ODrive. It stores the rising-edge time and uses `micros()` to calculate pulse width at the falling edge. At 115200 baud, it prints approximately every 20 ms:

```text
throttle_us    steering_us    engage_us
```

These are **the tester's output column names**, not a reconstructed measurement log.

Separating the channels provides a way to compare motion intent with motor-activation intent. The print interval does not capture every individual pulse fault, but the sketch establishes an observation point independent of motor response.

The existing project notes report these observations:

| Check | Recorded observation | Consequence for the investigation |
| --- | --- | --- |
| Arduino PWM inspection | Pulse-width jumps were observed | Balance control alone could not explain the problem |
| Connections and wiring | Connections/routing were reworked repeatedly | Inspect the input path separately |
| Receiver proximity to metal | Symptoms changed with metal proximity | Retain receiver/antenna placement as a cause candidate |
| Aluminum-foil experiment | Spikes became worse | No basis for adopting foil as the solution |

The foil experiment was an unsuccessful mitigation. It supported investigating conductive surroundings, but did not distinguish RF effects from PWM wiring noise.

## How input interpretation changed

Placement and close signal-ground routing were investigated as hardware mitigations. Software handling then addressed distinct input behaviors:

| Handling in the final sketch | Setting | Purpose |
| --- | --- | --- |
| Throttle/steering low-pass filtering | `alpha=0.4` | Reduce the effect of brief changes on motion requests |
| Engage low-pass filtering | `alpha=0.02` | Apply stronger smoothing to activation input |
| Neutral deadband | Zero input for deviations below `50 us` | Avoid interpreting small neutral drift as motion |
| Engage persistence | Counter updated with the 50 ms activation task | Reduce reliance on one activation decision |

The throttle recurrence is:

```cpp
filtered_throttle_pwm = kAlpha_1 * throttle_pwm + (1.0 - kAlpha_1) * filtered_throttle_pwm;
```

Engage uses a smaller coefficient than throttle/steering. The implementation distinguishes responsiveness of motion input from stability of activation decisions.

The persistence counter rises or falls, is constrained to `-3..3`, and declares engage when positive. It is therefore not a simple guarantee of exactly three consecutive samples before activation.

Neutral is not universally assumed to be 1500 us: the deadband centers are throttle 1488 us and steering 1492 us. Throttle normalization still uses 1491 us, so those references are not fully unified.

## The lesson was a criterion for trusting input

A received value is not automatically a valid operator intention. Neutral drift, a short spike, and an activation request need different treatment.

The investigation connected placement-dependent observations with channel-specific input processing. Hardware checks explained why the input path deserved attention; filters, deadband, and persistence reduced the influence of remaining variations.

## Evidence a reviewer can inspect

**Work demonstrated:** moving the observation point, interpreting an unsuccessful mitigation, and matching input handling to failure modes.

- [Receiver tester](../../../firmware/testers/receiver_pwm_test/receiver_pwm_test.ino): pulse-width measurement and output fields.
- [Physical controller](../../../firmware/physical_balance_controller/physical_balance_controller.ino): filter constants, deadband, and engage counter.
- [Development process](../../development-process.md): receiver and wiring investigation context.

Implementation is inspectable in code. Metal/foil outcomes come from existing project notes; raw logs and comparative spike rates are not published. This supports describing the response, not a numerical noise-reduction claim.
