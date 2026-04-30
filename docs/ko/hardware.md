# 하드웨어

[English](../hardware.md) | 한국어

이 문서는 실제 셀프 밸런싱 로봇의 단일 하드웨어 reference입니다. 기존 BOM, power/IO notes, layout interpretation, gallery material을 하나로 합쳐서 여러 작은 문서를 오갈 필요가 없게 했습니다.

<table>
  <tr>
    <td width="38%">
      <img src="../../media/hardware/robot_open_front.png" alt="Internal hardware photo" width="100%">
    </td>
    <td width="62%">
      <img src="../../media/diagrams/wiring_diagram.png" alt="Wiring Diagram" width="100%">
    </td>
  </tr>
  <tr>
    <td valign="top">
      <strong>Physical layout</strong><br>
      실제 chassis, sensor head, electronics bay, battery area, compute enclosure, wheel base입니다.
    </td>
    <td valign="top">
      <strong>System wiring summary</strong><br>
      Power chain, Arduino, BNO055, FrSky receiver, ODrive, motors, onboard PC, Gemini 330, auxiliary Arduino, relay path를 요약합니다.
    </td>
  </tr>
</table>

## 확인된 구성품

| Component | Identification | Role |
| --- | --- | --- |
| Main controller | Arduino Mega 2560 | 실제 balance, RC input, safety, ODrive command loop 실행 |
| IMU | BNO055 | 균형 제어를 위한 body angle 및 gyro feedback |
| Motor controller | ODrive 3.6 | Current-based dual-motor control |
| Depth camera | Orbbec Gemini 330 | ROS integration experiment를 위한 상단 sensor-head perception hardware |
| RC transmitter | FrSky 2.4GHz Taranis Q X7 | Handheld throttle, steering, engage control |
| RC receiver | FrSky X8R | Arduino로 들어가는 PWM command input |

## 보수적인 하드웨어 식별

일부 정확한 part number는 남아 있지 않기 때문에, firmware, wiring diagram, photo, process material을 바탕으로 보수적으로 설명했습니다.

| Subsystem | Best-effort identification | Why I think so |
| --- | --- | --- |
| Wheels and motors | Dual 36V hall-sensor BLDC hub motors, hoverboard-style 가능성이 높고 wheel diameter는 약 165 mm | Firmware가 hall feedback과 약 `0.0825 m` wheel radius를 사용하며, 사진도 hub-wheel assembly와 맞습니다 |
| Battery | 36V battery pack, lithium-ion 가능성이 높음 | Wiring diagram과 internal photo 모두 36V system bus를 보여줍니다 |
| Power conversion | 36V -> 19V 및 19V -> 5V DC-DC converters | Wiring diagram에 compute rail과 low-voltage control rail이 표시되어 있습니다 |
| Onboard compute | 19V mini PC, x86/NUC-class 가능성이 높음 | Wiring diagram에 PC rail이 있고, internal photo에 vented compute enclosure가 보입니다 |
| Auxiliary controller | Arduino Mini/Nano-class board | Wiring diagram에 5V side mini Arduino가 표시되어 있습니다 |
| Relay path | 5V relay 또는 switching module | Wiring diagram에 auxiliary relay/power-relay block이 보입니다 |
| Chassis | Metal base plate 위의 3D-printed body, mast, sensor-head mount | CAD/process image와 실제 사진이 purpose-built enclosure를 뒷받침합니다 |

## Power And Signal Flow

```text
36V Battery
  -> ODrive 3.6 -> dual wheel motors
  -> 36V->19V converter -> onboard PC
  -> 19V->5V converter -> auxiliary Arduino / relay module

FrSky Taranis Q X7 -> FrSky X8R receiver -> Arduino Mega 2560
BNO055 IMU -> Arduino Mega 2560
Arduino Mega 2560 -> ODrive 3.6 command path
Orbbec Gemini 330 -> onboard PC
```

실제 control path는 의도적으로 단순하게 유지했습니다: `FrSky receiver + BNO055 -> Arduino -> ODrive -> motors`. ROS와 perception hardware는 그 위 계층에 있으며, 실시간 balancing loop에는 필수 요소가 아닙니다.

## Layout Interpretation

공개 사진에는 직접적인 callout을 많이 넣지 않았습니다. 이전의 직접 라벨링은 부품을 과하게 특정할 위험이 있었기 때문입니다. 비교적 신뢰할 수 있는 visible zone은 다음과 같습니다.

- upper Orbbec Gemini 330 sensor-head area
- left-side auxiliary board area
- central compute and wiring bay
- right-side 36V battery pack
- internal electronics가 보이는 open front service area

## Historical Camera Naming Note

일부 simulation-side URDF와 launch file은 아직 `RealSense D435` asset이나 `d435` topic name을 참조합니다. 이것들은 historical simulation 또는 placeholder RGB-D workflow에 속합니다. 이 문서에서 실제 deployed physical camera로 기록하는 장비는 Orbbec Gemini 330입니다.

## 이 문서의 한계

- Wiring Diagram은 high-level project diagram이지 manufacturing schematic이 아닙니다.
- Wire color, connector pinout, regulator part number, 정확한 battery chemistry는 보이는 자료 이상으로 추정하지 않았습니다.
- 이 문서는 portfolio review와 system understanding을 위한 문서이며, 완전한 rebuild guide는 아닙니다.

## Supporting Images

| Asset | Why it matters |
| --- | --- |
| [Wiring Diagram](../../media/diagrams/wiring_diagram.png) | 주요 hardware, power, wiring overview |
| [Open-front robot photo](../../media/hardware/robot_open_front.png) | 실제 electronics packaging과 sensor-head layout |
| [Wheel bench test](../../media/process/wheel_bench_test.jpg) | full-body balancing 이전의 motor, wheel, controller bring-up |
| [Tethered driving practice](../../media/process/tethered_driving_practice.jpg) | 안전 지지 상태에서 진행한 physical tuning stage |
