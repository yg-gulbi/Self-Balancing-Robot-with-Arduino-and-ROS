# Public Media Catalog

This archive note maps recovered media into cleaner public titles and repo assets. Original phone or chat-export filenames are intentionally omitted so the repository reads like a finished project page instead of a raw staging folder.

## Public Assets

| Public title | Type | Public asset | Use in repository | Privacy decision |
| --- | --- | --- | --- | --- |
| Wiring sketch source | photo | `archive/wiring_hand_sketch.jpg`, `media/diagrams/wiring_diagram.png` | Converted into the clean public wiring diagram | Keep source as archive context only |
| Internal electronics view | photo | `media/hardware/robot_open_front.png` | Main physical hardware photo | Cropped and selected for public use |
| Robot-focused hallway still | photo | `media/demos/hallway_robot_only.jpg` | Supporting real-world driving image | Cropped to avoid operator-focused framing |
| Physical obstacle-course balancing demo | video/GIF | `media/demos/physical_balance_obstacle_course.gif` | Supporting balancing demo | Lightweight public GIF only |
| Physical hallway balancing demo | video/GIF | `media/hero/physical_balance_hallway.gif` | README hero demo | Lightweight public GIF only |
| Short-course and alternate raw clips | video | Not committed | Optional external clip after privacy review | Keep out of repo unless faces/operators are removed |
| Close-up setup clip | video | Not committed | Optional process clip after privacy review | Keep out of repo unless it adds clear technical value |

## Recovered Process Exports

These public images were exported from recovered process material and summarized in `docs/development-process.md`.

| Public image | Use |
| --- | --- |
| `media/process/catia_internal_assembly_views.jpg` | internal CATIA assembly views |
| `media/process/catia_head_design.jpg` | sensor-head and upper structure design |
| `media/process/catia_body_design.jpg` | body and internal packaging design |
| `media/diagrams/wiring_diagram.png` | public wiring diagram used across the hardware documentation |
| `media/process/wheel_bench_test.jpg` | wheel and electronics bench-test stage |
| `media/process/tethered_driving_practice.jpg` | tethered physical driving practice |
| `media/process/simulation_unreal_navigation_views.jpg` | ROS/Gazebo simulation and navigation views |

## Selection Notes

- The hallway GIF is used as the hero because it shows the clearest continuous balancing run.
- The obstacle-course GIF is used to show controlled physical motion around simple obstacles.
- Full raw clips should stay outside the Git repository unless they are trimmed, anonymized, and clearly useful to the technical story.
