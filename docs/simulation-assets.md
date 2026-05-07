# Simulation Asset Baseline

This note records the actual robot-derived geometry assets that were recovered
while preparing the repository for GitHub.

## Source Archive

The recovered files came from the workspace archive:

- `archive_sources/legacy_workspace_archive/exported_balance_robot_stl_changes_20260507`

That archive contained:

- `stl/`: body-part STL exports
- `urdf/balence_robot_urdf.xacro`
- `urdf/balence_robot_urdf_depth.xacro`
- supporting README notes about missing single-mesh placeholders

## What Was Moved Into The Repo

The GitHub-facing package is:

- `ros_ws/src/balance_robot_description/`

Recovered visual assets now live in:

- `ros_ws/src/balance_robot_description/stl/body_side.stl`
- `ros_ws/src/balance_robot_description/stl/robot_neck.stl`
- `ros_ws/src/balance_robot_description/stl/head_rear.stl`
- `ros_ws/src/balance_robot_description/stl/head_top.stl`
- `ros_ws/src/balance_robot_description/stl/gemini335_case.stl`

The active LiDAR and depth Xacros now expose:

- `use_actual_stl_assembly:=true`

That flag swaps only the `base_link` visuals. Collision and inertia still stay
simple so the historical controller and navigation behavior do not change just
because the model looks closer to the real robot.

## Why This Matters

These assets are the clearest geometry baseline for future simulation cleanup
because they were exported from the actual robot build direction rather than
being only a generic box approximation.

This means future simulation edits can reference:

- the multi-part body silhouette
- the Gemini 335 case placement
- the archived visual assembly origin used in the original workspace

## Archived Frame Notes Worth Reviewing Later

The archived depth-model Xacro used these visual and frame values:

- STL assembly origin: `-0.110383 -0.202 0.064314`
- IMU joint origin: `0 0 0.49`
- camera joint origin: `0.0515 0.0 0.400`

The current repository still keeps its later simplified collision model and its
current sensor-frame placements unless you explicitly change them. That is
intentional for now, because this cleanup was about organizing the repo and
recovering the real-robot geometry baseline without silently retuning the
simulation behavior.

## Recommended Next Step For Simulation Edits

When you start modifying the simulation code itself, review these files first:

1. `ros_ws/src/balance_robot_description/urdf/balance_robot_urdf.xacro`
2. `ros_ws/src/balance_robot_description/urdf/balance_robot_urdf_depth.xacro`
3. `ros_ws/src/balance_robot_description/stl/README.md`
4. `archive_sources/legacy_workspace_archive/exported_balance_robot_stl_changes_20260507/urdf/balence_robot_urdf_depth.xacro`

That gives you the current GitHub baseline, the recovered real-robot visuals,
and the archived frame data in one place.
