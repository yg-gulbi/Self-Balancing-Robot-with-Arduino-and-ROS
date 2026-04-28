# Media Manifest

This file maps the raw staging assets to their public portfolio use.

| Raw filename | Type | Interpreted content | Publish decision | Public destination |
| --- | --- | --- | --- | --- |
| `KakaoTalk_20260424_163858019.jpg` | photo | hand-drawn power and signal sketch | publish | `media/hardware/wiring_hand_sketch.jpg`, `media/diagrams/hardware_power_io_overview.svg` |
| `KakaoTalk_20260424_163858019_01.jpg` | photo | front-open robot photo showing internal electronics | publish raw photo | `media/hardware/robot_open_front.png` |
| `KakaoTalk_20260424_164011837.jpg` | photo | hallway driving scene with operator present in original frame | publish cropped only | `media/demos/hallway_robot_only.jpg` |
| `KakaoTalk_20260424_164016018.mp4` | video | short obstacle or arena driving clip | external only | upload later as full MP4 |
| `KakaoTalk_20260424_164020816.mp4` | video | obstacle-course physical balance drive | publish as supporting GIF + external original | `media/demos/physical_balance_obstacle_course.gif` |
| `KakaoTalk_20260424_164021546.mp4` | video | hallway physical balance drive | publish as hero GIF + external original | `media/hero/physical_balance_hallway.gif` |
| `KakaoTalk_20260424_164024601.mp4` | video | doorway or short-range drive clip | external only | upload later as full MP4 |
| `KakaoTalk_20260424_164024928.mp4` | video | alternate hallway driving clip | external only | upload later as full MP4 |
| `KakaoTalk_20260424_164028983.mp4` | video | close-up handling or setup clip | external only | upload later as full MP4 |

## Recovered Process Exports

These public-safe images were exported from recovered process material and summarized in `docs/development_process.md`.

| Source material | Public image | Use |
| --- | --- | --- |
| final presentation | `media/process/catia_concept_overview.jpg` | concept and overall CATIA model |
| weekly presentation | `media/process/catia_internal_assembly_views.jpg` | internal CATIA assembly views |
| final presentation | `media/process/catia_head_design.jpg` | sensor-head and upper structure design |
| final presentation | `media/process/catia_body_design.jpg` | body and internal packaging design |
| circuit planning deck | `media/process/source_wiring_block_diagram.jpg` | source block diagram for public wiring explanation |
| wheel bench-test video | `media/process/wheel_bench_test.jpg` | wheel and electronics bench-test evidence |
| final presentation | `media/process/tethered_driving_practice.jpg` | tethered physical driving practice |
| final presentation | `media/process/simulation_unreal_navigation_views.jpg` | ROS/Gazebo simulation and navigation views |

## Selection Notes

- Hero clip was chosen for the clearest continuous hallway driving with no visible operator in the public GIF.
- Supporting obstacle-course clip was chosen for showing controlled physical motion around simple obstacles.
- The original hallway photo was cropped for public use to avoid keeping the uncropped operator scene in the repo-facing assets.
