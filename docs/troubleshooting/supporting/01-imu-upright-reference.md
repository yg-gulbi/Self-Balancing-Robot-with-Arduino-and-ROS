# IMU zero was not the robot's upright reference

[한국어](../../ko/troubleshooting/supporting/01-imu-upright-reference.md) | [All troubleshooting](../../troubleshooting.md)

> Sensor calibration and mechanical upright reference were treated separately. The final attitude calculation applies an explicit offset.

## Persistent correction could involve the reference, not only gains

If the IMU angle differs from the useful physical upright posture, the controller continually corrects toward the wrong point. This motivated calibration/reference management in the existing project notes. Sensor startup state, mounting direction, and physical equilibrium therefore needed attention before gain changes alone could explain the behavior.

## Sensor state and upright reference were separated

The sketch halts on BNO055 initialization failure and provides calibration-status and EEPROM save/load helpers. A separate `imu_angle_offset` adjusts the control reference.

| Code path | What it checks or changes |
| --- | --- |
| `bno.begin()` | Sensor initialization |
| `checkIMUCalibration()` | System/gyro/accel/magnetometer calibration status |
| `saveCalibration()` / `loadCalibration()` | Store/restore 22 bytes of calibration data |
| `SetImuAngleOffset()` | Adjust the posture reference used by control |

Sensor calibration data and body upright offset are different quantities. Calibrating the sensor does not remove every mounting/equilibrium difference.

## What is active in the final sketch

```cpp
float imu_angle_offset = 3.5;
```

The angle calculation is:

```cpp
theta = imu_enabled * (euler_angles.y() + imu_angle_offset);
```

With IMU enabled, corrected zero corresponds to raw y-angle -3.5 degrees. **That is a code-derived reference, not a measured posture log.** The tilt-threshold check uses the same offset.

The startup calls to `loadCalibration()` and `checkIMUCalibration()` are commented out. Helper availability therefore does not establish automatic restoration/status reporting on each boot.

## The reference became an explicit input condition

Offset adjustment is exposed separately through serial key `r`. This distinguishes insufficient recovery response from a different intended equilibrium point.

## Evidence a reviewer can inspect

**Work demonstrated:** distinguishing sensor initialization, calibration, and mechanical reference; explicit control-reference configuration.

Inspect `setup()`, `checkIMUCalibration()`, `SetImuAngleOffset()`, and `BalanceController()` in the [physical controller](../../../firmware/physical_balance_controller/physical_balance_controller.ino). Offset implementation is inspectable; fixed-posture restart logs are not published, so a numerical repeatability improvement is not established.
