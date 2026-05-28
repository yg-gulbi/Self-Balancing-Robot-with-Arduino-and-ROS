# 기구 3D 파일

[English](README.md) | 한국어

이 폴더는 로봇 골격, 출력용 body part, sensor-head 구조를 GitHub에서
찾아보기 쉽게 정리한 3D 파일 모음입니다.

파일은 2026-05-11에 로컬 모델링 archive에서 가져왔습니다.

- `D:\06_3D_모델링\SLDPRT\balancing robot`
- `D:\06_3D_모델링\SLDPRT\new_balencerobot`

CAD assembly와 과거 작업 흔적을 추적하기 쉽도록 원본 파일명은 가능한 한
그대로 보존했습니다. `balanicng`, `balence`처럼 오타처럼 보이는 이름도
원본 파일명과 연결되도록 일부러 바꾸지 않았습니다.

## 폴더 역할

| 경로 | 내용 | 용도 |
| --- | --- | --- |
| `print_ready/` | main body package, side plate, board plate, battery plate, breadboard plate, Intel plate, resistor plate의 STL 및 CATPart export | 3D printing reference 및 수정 가능한 part source |
| `head_sensor_mount/` | neck, inner neck, head versions, Gemini case file(`gemini335_case`), LiDAR case의 CATIA/STL 파일 | sensor-head와 상단 frame 참고 |
| `new_balancerobot_concept/` | 작은 SolidWorks concept assembly, SLDPRT part, STL, STEP export | 이후 concept/reference geometry |

## 시각 참고 이미지

아래 eDrawings 이미지는 source CAD의 기준이 된 `balancing robot_ver4`
assembly view입니다.

<table>
  <tr>
    <td width="50%">
      <img src="../media/process/edrawings_balancing_robot_ver4_assembly.png" alt="Balancing robot ver4 eDrawings assembly view" width="100%">
    </td>
    <td width="50%">
      <img src="../media/process/edrawings_balancing_robot_ver4_exploded.png" alt="Balancing robot ver4 eDrawings exploded view" width="100%">
    </td>
  </tr>
</table>

## ROS 모델과의 관계

현재 ROS/Gazebo package는 `ros_ws/src/balance_robot_description/stl/`의
정리된 mesh set을 사용합니다. 그쪽 파일은 URDF/Xacro에서 참조하기 쉽도록
짧은 이름으로 정리한 simulation-friendly visual asset입니다.

실제 제작용 source file이나 physical frame design을 확인할 때는 이
`mechanical/` 폴더를 보면 됩니다. URDF/Xacro visual을 수정할 때는 ROS
package 안의 `stl/` 폴더를 기준으로 보는 것이 좋습니다.

## 출력 전 메모

- 출력 전 slicer에서 scale, orientation, wall thickness, hole tolerance를
  다시 확인해야 합니다.
- 특정 printer용 G-code는 넣지 않았습니다. printer, nozzle, material,
  layer height에 맞춰 새로 생성하는 편이 안전합니다.
- 현재 ROS 모델의 collision과 inertia는 의도적으로 단순화되어 있습니다.
  이 상세 mesh를 simulation physics에 바로 넣으려면 mass, origin,
  collision assumption을 먼저 검토해야 합니다.
