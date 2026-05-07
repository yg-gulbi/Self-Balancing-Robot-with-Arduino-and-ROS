# STL Visual Assets

These STL files are the cleaned, GitHub-ready body-part exports recovered from
the actual robot-based simulation archive on 2026-05-07.

Tracked files:

- `body_side.stl`
- `robot_neck.stl`
- `head_rear.stl`
- `head_top.stl`
- `gemini335_case.stl`

The original CATIA-style export names remain in the workspace archive. This
folder keeps the shorter names that are easier to reference from URDF/Xacro and
easier to browse on GitHub.

These meshes are used for visual fidelity only. Collision and inertia remain
simple in the active robot models so Gazebo control and navigation behavior do
not shift unexpectedly.
