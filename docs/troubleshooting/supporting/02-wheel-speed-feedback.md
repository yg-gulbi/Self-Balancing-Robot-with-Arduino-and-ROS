# A speed spike changes motor correction: follow the feedback handling

[한국어](../../ko/troubleshooting/supporting/02-wheel-speed-feedback.md) | [All troubleshooting](../../troubleshooting.md)

> Archived code uses recent history and a median fallback for abrupt samples. The final sketch uses receive/parsing handling and bounded speed correction. Historical experiments and active processing are distinguishable.

## A feedback fault can affect the common motor effort

The speed loop subtracts mean wheel speed from target speed. Its correction enters both motor requests. One abnormal sample can therefore change the motor correction even without an operator-input change.

The question was **which samples should influence control**, not simply how to print wheel speed.

## Earlier code compared samples with recent history

The [legacy controller](../../../archive/arduino_firmware/legacy_balance_controller.ino) computes a threshold using the mean absolute magnitude of five recent values:

```cpp
return 3.0 * (sum / N);
```

An abrupt change beyond that threshold is replaced with the median of the new sample and two previous values:

```cpp
if (abs(phi_dot_counts_0 - prev_dot_0) > adaptive_threshold_dot_0) {
    phi_dot_counts_0 = median_phi_dot_0;
}
```

This experiments with history-dependent checks and a median fallback. The new sample also enters the threshold history; this is not evidence of a validated statistical outlier detector.

## Active processing differs from the archived experiment

| Path | Archived code | Final sketch |
| --- | --- | --- |
| Abrupt changes | History threshold and three-value median | Same path absent from the active loop |
| Receive handling | Position/speed inspection paths | Drain available bytes around two `parseFloat()` reads |
| Speed smoothing | Historical equations/settings | Recurrence retained, but `kAlpha_3=1` |
| Correction bounds | Historical control settings | Integral +/-5; speed output +/-6 |

At `kAlpha_3=1`, each new speed sample passes directly through. A variable called `filtered_phi_dot` does not establish active smoothing.

A `filterEncoderData()` helper also remains in the final file but is not called by active `BalanceController()`. Draining available bytes does not validate complete response framing, sample validity, or timeout outcomes.

## Measurement validity and output bounds solve different problems

Integral/output limits constrain correction magnitude. Outlier detection judges the sample itself. One does not establish the other.

Comparing versions shows feedback-processing experiments, while preventing the claim that every archived countermeasure is deployed.

## Evidence a reviewer can inspect

**Work demonstrated:** tracing measurement errors into control effort, experimenting with sample handling, and checking the active code path.

- [Legacy controller](../../../archive/arduino_firmware/legacy_balance_controller.ino): `updateThreshold()`, `getMedian()`, abrupt-sample replacement.
- [Final controller](../../../firmware/physical_balance_controller/physical_balance_controller.ino): receive/parsing, `kAlpha_3`, speed bounds.

The code establishes changes in processing. Synchronized raw responses, wheel speed, and current-request comparison logs are not published, so numerical vibration reduction is not claimed.
