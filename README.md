# Mage Squad: 우리들의 마법 생존기  
> Unreal Engine 기반 생존 액션 게임 프로젝트 – Team Rise

## 🎮 프로젝트 개요
**Mage Squad: 우리들의 마법 생존기**는 마법을 활용해 적의 파도를 버티며 성장하는 3D 생존 액션 게임입니다.  
팀 Rise는 Git 기반 협업 환경에서 개발을 진행하며, Perforce 환경 안정화 시 git p4를 통한 이전도 검토할 예정입니다.

게임 레퍼런스  
https://store.steampowered.com/app/2904000/_/?l=koreana

## ⚙️ 개발 환경
- 엔진: Unreal Engine  
- 언어: C++ / Blueprint  
- IDE: Visual Studio  
- 버전 관리: Git (Git Flow 기반)  
- 협업 도구: JIRA(일정·업무 관리), Discord(커뮤니케이션)

## 👥 협업 규칙

### 🔹 커밋 규칙
`<타입>: <메시지>` 형식을 사용합니다.
```
NEW: 신규 기능
CHG: 코드 변경
FIX: 문제 수정
```

### 브랜치 전략 (Git Flow)
- main  
  - 실제 서비스가 가능한 안정(stable) 버전 유지  
  - 릴리즈 시점에만 업데이트  
- develop  
  - 모든 개발 작업의 기준 브랜치  
  - 작업 시작 시 항상 최신 develop을 가져와 진행  
- feature/\*  
  - 기능 개발용 브랜치  
  - 형식: feature/<기능명>

## 🔧 코드 컨벤션

### 기본 규칙
- 언리얼 엔진 공식 코드 컨벤션을 따릅니다.  
- 클래스 및 내부 구조에는 MS 접두어 규칙을 적용합니다.

### 클래스 주석 작성 규칙
```cpp
/*
* 작성자: 임희섭
* 작성일: 25/11/03
* 클래스 간단 설명 (ex. 플레이어 점멸(Blink)를 위한 컴포넌트)
*/
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TEST_5_6_API UBlinkComponent : public UActorComponent
{
    ...
};