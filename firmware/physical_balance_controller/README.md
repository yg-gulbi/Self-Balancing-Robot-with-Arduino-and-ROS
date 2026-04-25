# Physical Balance Controller

This folder contains the canonical physical robot controller for the portfolio repository.

- Main file: [`physical_balance_controller.ino`](physical_balance_controller.ino)
- Purpose: real-world balancing and RC driving on Arduino using BNO055 and ODrive

Why this file is the main controller:

- It is the clearest standalone physical robot balancing implementation in the archive.
- It keeps the balancing and drive loop on Arduino rather than requiring ROS in the low-level loop.
- It best matches the actual completed physical robot behavior claimed by this repository.
