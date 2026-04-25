# Timeline

## High-Level Evolution

1. Early Gazebo balancing experiments
2. Separation of navigation command and balancing command through `/before_vel`
3. PID tuning and auto-tuning work in simulation
4. Camera/SLAM/navigation integration experiments
5. Portfolio-oriented cleanup and archive recovery

## Workspace Interpretation

- Early exploration was spread across multiple workspaces.
- The strongest cleaned simulation baseline came from the `balance-robot-sim` lineage.
- Physical robot control evolved through several Arduino sketches before the main controller version was stabilized.
- Real-world ROS integration work existed, but not all workspaces reached the same maturity level.
