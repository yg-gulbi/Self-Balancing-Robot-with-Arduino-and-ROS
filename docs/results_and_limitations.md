# Results And Limitations

## Confirmed Results

- Real physical self-balancing control on Arduino
- Manual RC driving while balancing
- ROS/Gazebo balancing robot simulation
- Simulation-side SLAM and navigation workflows
- RC-to-ROS command bridge testing
- ODrive, IMU, RC, and controller bring-up experiments

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

For the full process narrative, see [build_story.md](build_story.md).

## Current Gaps

- Some historical notes remain as raw archive material
- Third-party dependency versions should be pinned more tightly in a future cleanup
- Original full-length MP4 clips are tracked externally instead of stored directly in the repository
