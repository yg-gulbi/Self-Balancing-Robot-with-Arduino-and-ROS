# Robot Description Package

This package keeps the active URDF/Xacro files for the balancing robot and the
recovered STL body parts that came from the actual robot-based simulation
archive.

## Folder Roles

- `urdf/`: active LiDAR and depth-camera robot models used by launch files
- `stl/`: recovered body-part visuals copied from the 2026-05-07 archive export

## Visual Modes

By default, the robot body stays a simple box. That keeps the historical
simulation behavior, collision volumes, and inertia assumptions stable.

If you want the recovered actual-robot body visuals, launch with:

```bash
roslaunch robot_bringup robot_remote_depth.launch use_actual_stl_assembly:=true
```

or:

```bash
roslaunch balance_robot_workflows robot_navigation_lidar.launch use_actual_stl_assembly:=true
```

`use_actual_stl_assembly:=true` switches the `base_link` visuals to the
multi-part STL assembly in `stl/`, but collision and inertia stay simple on
purpose.

## Notes

- The STL placement transform comes from the archived simulation workspace
  that was derived from the physical robot.
- The archive also referenced a single-file `meshes/base_body.stl`, but that
  mesh was not recovered here. The tracked GitHub baseline is the multi-part
  `stl/` assembly instead.
- For the archive provenance and the frame-position notes worth reviewing
  before changing simulation code, see `../../../docs/simulation-assets.md`.
