# IMU calibration and upright reference

[한국어](../../ko/troubleshooting/supporting/01-imu-upright-reference.md) | [Troubleshooting index](../../troubleshooting.md)

**Priority:** Supporting · Reconstructed from existing documentation and code

## 1. Problem definition

A sensor's zero angle can differ from the robot's useful upright posture. Mounting bias and calibration state can therefore produce persistent correction or creep; the old document describes these effects but publishes no isolated drift dataset.

## 2. Candidate solutions

Check sensor detection and calibration status; distinguish sensor calibration from mechanical upright-offset adjustment. Consider EEPROM persistence for repeatability, but verify whether loading is actually enabled.

## 3. Execution

The final sketch halts when BNO055 initialization fails, provides calibration save/load helpers and status inspection, and computes body angle using `imu_angle_offset=3.5`. The startup `loadCalibration()` call is commented out. Helper availability therefore does not demonstrate automatic restoration on every boot.

## 4. Reinterpreting the experience

Upright is an operating reference that must agree with sensor mounting and robot mechanics. Calibration helpers, enabled startup behavior, and a suitable posture offset are separate pieces of evidence.

## 5. Summary and verification

Outcome: explicit angle-offset compensation is active; EEPROM helpers exist, but startup restoration is disabled in this sketch. Review `setup()`, `saveCalibration()`, `loadCalibration()`, and the angle calculation. A future restart test should log calibration status and angle at a fixed physical posture.

### Evidence

- [Physical controller](../../../firmware/physical_balance_controller/physical_balance_controller.ino)
