# Physical Balance Controller

This folder contains the canonical physical robot controller for the portfolio repository.

- Main file: [`physical_balance_controller.ino`](physical_balance_controller.ino)
- Control structure: [`control_algorithm.md`](control_algorithm.md)
- Purpose: real-world balancing and RC driving on Arduino using BNO055 and ODrive

Why this file is the main controller:

- It is the clearest standalone physical robot balancing implementation in the archive.
- It keeps the balancing and drive loop on Arduino rather than requiring ROS in the low-level loop.
- It best matches the actual completed physical robot behavior claimed by this repository.

## What The Controller Does

At a high level, the controller combines:

- RC PWM input for throttle, steering, and motor engage
- BNO055 tilt angle, angular velocity, and yaw-rate feedback
- ODrive wheel-speed feedback
- LQR-style balance correction, PID-like speed correction, and steering correction
- motor safety gates, tilt cutoff, current limiting, and serial gain tuning

See [`control_algorithm.md`](control_algorithm.md) for the GitHub-friendly algorithm diagram and control-flow explanation.
