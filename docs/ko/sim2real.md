# Sim2Real Bridge

[English](../sim2real.md) | 한국어

이 문서는 simulation과 실제 로봇을 어떻게 연결해서 사용했는지 설명합니다. 일부러 보수적으로 정리했습니다. 이 프로젝트는 실제 로봇의 self-balancing 및 RC driving을 완료했고, ROS/Gazebo navigation workflow는 simulation에서 완료했습니다. 실제 balancing robot에서 full autonomous ROS navigation은 future work로 남겨 둡니다.

## Core Idea

가장 중요한 sim-to-real 아이디어는 high-level motion command가 balancing layer를 우회하지 않게 만드는 것이었습니다.

```text
Simulation:
teleop or move_base
  -> /before_vel
  -> balance_robot_control
  -> /cmd_vel
  -> Gazebo robot

Physical robot:
RC or ROS-side intent
  -> Arduino balance and safety loop
  -> ODrive current commands
  -> wheel motors
```

Simulation에서는 `/before_vel`이 motion intent topic입니다. Balancing controller가 그 intent를 받아 final `/cmd_vel`로 바꿉니다. 실제 로봇에서는 Arduino가 더 낮은 레벨에서 같은 safety role을 맡습니다. Throttle, steering, ROS-side intent가 로봇에 움직임을 요청할 수는 있지만, IMU feedback, tilt cutoff, current clamp, wheel-speed correction을 거친 뒤에야 motor command가 결정됩니다.

## What Transferred From Simulation To Hardware

| Transfer point | Simulation side | Physical side |
| --- | --- | --- |
| Robot form factor | Gazebo model이 같은 two-wheeled layout과 3D-printed design direction을 따름 | 실제 로봇은 printed chassis, upper sensor structure, internal electronics packaging을 사용 |
| Command layering | Navigation과 teleop이 balance control 전에 intent를 publish | RC와 ROS bridge path를 local Arduino balance loop로 들어가는 command로 처리 |
| Balance-aware motion | Desired velocity를 final motion output 전에 lean 또는 correction request로 변환 | RC throttle을 wheel speed direct command가 아니라 balance point를 이동시키는 speed target으로 사용 |
| Sensor workflow | Gazebo가 `/imu`, `/odom`, scan/depth, map, RViz feedback으로 workflow test 제공 | BNO055, ODrive feedback, RC PWM, camera integration trace가 실제 sensing path를 구성 |
| Safety boundary | Simulation controller가 pitch가 unsafe해지면 output을 stop 또는 limit | Firmware에 tilt cutoff, engage persistence, current clamp, inactive-state reset 존재 |

## What Stayed Simulation-Only

ROS navigation stack은 실제 로봇보다 Gazebo에서 더 깨끗하게 완성되었습니다. Simulation에서는 control, LiDAR navigation, depth navigation, SLAM, PID tuning launch file이 정리되어 있습니다. 실제 로봇에서 가장 강한 completed result는 Arduino-controlled balancing과 RC driving입니다.

이 분리는 중요합니다. Balancing robot은 일반 differential-drive base가 아닙니다. Planner가 velocity command를 출력하더라도 robot은 body angle, current limit, recovery behavior를 계속 지켜야 합니다. Simulation은 command architecture와 workflow composition을 증명했고, 실제 로봇은 low-level balancing과 driving behavior를 증명했습니다.

## What Needed Real-World Tuning

| Area | Why simulation was not enough | Repository evidence |
| --- | --- | --- |
| IMU angle and gyro behavior | 실제 calibration, mounting offset, noise가 balance loop에 영향 | [`physical_balance_controller.ino`](../../firmware/physical_balance_controller/physical_balance_controller.ino), [`control_algorithm.md`](../../firmware/physical_balance_controller/control_algorithm.md) |
| RC input reliability | Receiver PWM value가 drift될 수 있어 filtering, deadband, engage persistence 필요 | [`troubleshooting.md`](troubleshooting.md) |
| Motor and wheel feedback | Hall feedback, current response, wheel behavior에는 실제 hardware limit 존재 | [`motor_current_test.ino`](../../firmware/testers/motor_current_test/motor_current_test.ino), [`hardware.md`](hardware.md) |
| Fall safety | Physical testing에서는 robot 보호를 위해 current clamp와 tilt cutoff 필요 | [`results-and-limitations.md`](results-and-limitations.md) |
| Depth-camera integration | Camera, TF, SLAM, navigation trace는 있었지만 full physical autonomy는 미완료 | [`real-world integration archive`](../../archive/ros_experiments/real_world_integration/README.md) |

## Honest Result

이 프로젝트는 practical sim-to-real bridge를 두 방향으로 보여줍니다.

- simulation to hardware: balance-aware command separation, navigation workflow structure, robot layout, tuning idea
- hardware back to simulation: safety constraint, real controller limit, final actuator authority를 high-level planner 아래에 둬야 한다는 점

완료된 physical result는 balancing과 RC driving입니다. 완료된 ROS result는 simulation-side control, SLAM/navigation workflow composition, command-path adaptation입니다. 아직 끝나지 않은 bridge는 실제 balancing robot에서 full end-to-end autonomous navigation을 수행하는 부분입니다.
