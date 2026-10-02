# 실물 밸런스 튜닝과 전도 위험 관리

[English](../../../troubleshooting/major/03-balance-tuning.md) | [트러블슈팅 목차](../../troubleshooting.md)

**분류:** 중요 이슈 · 기존 문서와 코드에서 재구성한 기록

## 1. 문제정의

게인, 직립 기준, 바퀴 피드백, 모터 응답의 작은 변화가 복원 또는 전도로 이어졌다. 완성 로봇에서 바로 시험하면 여러 원인이 섞이므로 게인만 조정해서는 실패 원인을 구분하기 어려웠다.

## 2. 해결방법 안들 설정

부품별 동작 확인과 자세 제어 튜닝을 분리한다. 모터·바퀴 벤치 시험에서 시작하고 지지줄을 사용한 시험 뒤 자유 주행으로 진행한다. 각도·각속도 피드백에 속도와 조향 보정을 결합하고 출력과 적분 누적을 제한한다. 단계적 시험 근거는 있지만 전체 게인 탐색 순서가 기록된 것은 아니다.

## 3. 해결방법 실행

최종 자세 보정식은 `(K_theta/100)*theta - (K_theta_dot/100)*theta_dot + (Ki/100)*integral_error`이다. 설정은 `K_theta=24`, `K_theta_dot=1.7`, `Ki=0`이므로 각도 적분 경로는 존재하지만 현재 출력에는 기여하지 않는다. 속도 보정은 `Kp=1.2`, `Ki=0.1`, `Kd=7`이며 적분과 출력을 제한한다. 적분·차분은 갱신마다 계산하고 명시적인 `dt` 보정은 없다. 조향 항은 두 전류 명령에 반대 부호로 혼합한다. 기울기 임계값은 30°, 전류 명령 제한은 ±8 A이고 벤치·지지줄 시험 사진이 남아 있다. 보호 구조물 접촉 각도가 약 28°라는 기록이 있으므로 30° 차단을 접촉 전 보호 보장으로 설명하면 안 된다.

## 4. 경험의 재해석

튜닝의 대상은 게인뿐 아니라 측정 신뢰도, 출력 한계, 구동 가능한 상태까지 포함한다. 최종 구현은 자세·속도·조향 보정을 결합한 계층형 제어기다. 식과 설정만으로 최적 LQR 설계나 고정 주기 연속시간 PID 구현을 입증하지는 않는다.

## 5. 정리

프로젝트 전체 결과로 약 1시간 균형 유지와 10 m 복도·장애물 주행이 보고되어 있고 결과 문서와 영상이 있다. 이 자료가 각 게인이나 보호 로직의 개별 효과를 입증하는 것은 아니다. 제어 코드·알고리즘 문서·지지줄 시험과 주행 영상을 검토한다. 반복 튜닝에는 pitch, 바퀴 속도, 루프 시간, 전류 명령, 게인, 전도·정지 판정 기준을 함께 기록해야 한다.

### 검증 근거

- [Physical controller](../../../../firmware/physical_balance_controller/physical_balance_controller.ino)
- [Control algorithm](../../../../firmware/physical_balance_controller/control_algorithm.md)
- [Build process](../../../../docs/development-process.md)
- [Results and limitations](../../../../docs/results-and-limitations.md)
- [Tethered practice](../../../../media/process/tethered_driving_practice.jpg)
- [Hallway demo](../../../../media/hero/physical_balance_hallway.gif)
