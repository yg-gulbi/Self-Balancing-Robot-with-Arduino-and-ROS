# 트러블슈팅 목차

[English](../troubleshooting.md) | 한국어

이 문서는 개별 이슈를 찾아가는 목차다. 각 문서는 **문제정의 → 해결방법 안들 설정 → 해결방법 실행 → 경험의 재해석 → 정리** 순서로 작성했다. 중요도는 안전·핵심 동작에 미치는 영향과 조사 범위를 기준으로 나눴으며, 보조 이슈도 제어 안정성에 영향을 준다.

## 중요 트러블슈팅

| 문서 | 다루는 문제 |
| --- | --- |
| [RC PWM 신호 튐과 의도하지 않은 바퀴 움직임](troubleshooting/major/01-rc-pwm-spikes.md) | 입력 신호 튐과 오조작 방지 |
| [ODrive 폭주와 비정상 모터 동작](troubleshooting/major/02-odrive-runaway.md) | 모터 경로 분리와 다운그레이드 결과 |
| [실물 밸런스 튜닝과 전도 위험 관리](troubleshooting/major/03-balance-tuning.md) | 단계적 시험과 자세·속도·조향 제어 |

## 보조 트러블슈팅

| 문서 | 다루는 문제 |
| --- | --- |
| [IMU 캘리브레이션과 직립 기준](troubleshooting/supporting/01-imu-upright-reference.md) | 센서 보정과 실제 직립 기준 |
| [바퀴 속도 피드백과 시리얼 수신 신뢰도](troubleshooting/supporting/02-wheel-speed-feedback.md) | 이상치·파싱·필터 활성 여부 |
| [ROS 주행 명령 경로와 밸런스 제어 권한](troubleshooting/supporting/03-ros-command-path.md) | 이동 의도와 최종 제어 출력 분리 |

## 근거와 읽는 기준

- **코드로 확인:** 현재 구현과 설정을 직접 확인할 수 있다. 함수가 존재하는 것과 실행되는 것은 구분한다.
- **기존 기록의 보고:** 당시 문서에 결과가 기록되어 있지만 원시 로그·정량 비교가 없을 수 있다.
- **추론:** 관찰과 맞는 설명이며 원인 확정으로 표현하지 않는다.
- **후속 검증:** 각 문서의 확인 절차는 검토 수단이다. 이번 문서 재작성 중 실물 시험을 다시 수행한 것은 아니다.

해결안 목록은 기존 자료에서 재구성했다. 기록에 없는 실험 순서·측정값·실패 횟수를 새로 만들지 않았다. 필터와 안전 로직은 실제 문제를 다루는 문서 안에 배치하고, 상세 제어 수식은 [제어 알고리즘](../../firmware/physical_balance_controller/control_algorithm.md)에서 확인한다.

## 관련 자료

- [개발 과정](development-process.md)
- [결과와 한계](results-and-limitations.md)
- [Sim2Real](sim2real.md)
