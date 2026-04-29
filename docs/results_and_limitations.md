# Results And Limitations

## Confirmed Results

- Real physical self-balancing control on Arduino
- Manual FrSky radio driving while balancing
- ROS/Gazebo balancing robot simulation
- Simulation-side SLAM/navigation launch workflow
- RC-to-ROS command bridge testing
- ODrive 3.6, BNO055, FrSky Taranis Q X7 / X8R, Orbbec Gemini 330, and controller bring-up experiments

## Not Claimed

- Verified end-to-end autonomous ROS navigation on the physical balancing robot
- Fully reproducible real-world hardware setup from the current cleaned repository alone
- Complete preservation of every historical workspace variant

## Why This Matters

This repository is written for hiring review. It prioritizes accurate scope over inflated claims. The value of the project is in the full-stack robotics process:

- low-level embedded control
- actuator and sensor integration
- ROS/Gazebo simulation design
- navigation pipeline adaptation for a balancing robot
- iterative experimentation and recovery of older work

For the full process narrative, see [development_process.md](development_process.md).

## Current Gaps

- Historical code and notes are kept out of the main reading path under `archive/`
- Third-party dependencies are documented, but not vendored into the repository
- Original full-length MP4 clips are intentionally replaced by lightweight public-safe GIFs and stills
