Mage Squad: 우리들의 마법 생존기

Unreal Engine 기반 생존 액션 게임 프로젝트 – Team Rise

🎮 프로젝트 개요

Mage Squad: 우리들의 마법 생존기는 마법을 활용해 몰려오는 적들을 버티고 성장하는
3D 생존 액션 게임입니다.

팀 Rise는 Git 기반 협업 환경에서 개발을 진행하며,
필요 시 안정화된 시점에 Perforce로 이전하는 것을 목표로 하고 있습니다.

게임 레퍼런스
https://store.steampowered.com/app/2904000/_/?l=koreana

⚙️ 개발 환경

엔진: Unreal Engine

언어: C++ / Blueprint

IDE: Visual Studio

버전 관리: Git (Git Flow 기반)

협업 도구: JIRA(일정·업무 관리), Discord(커뮤니케이션)

👥 협업 규칙
🔹 브랜치 전략 (Git Flow)

main

실제 서비스 가능한 안정(stable) 버전 유지

릴리즈 시점에만 업데이트

develop

모든 개발 작업의 기준이 되는 브랜치

최신 develop 기반으로 작업 시작

feature/

기능 개발용 브랜치

형식: feature/<기능명>

🔧 코드 컨벤션
🔹 기본 규칙

언리얼 엔진 공식 코드 컨벤션을 따릅니다.

접두어 규칙은 팀 내부 규칙에 따라 MS를 사용합니다.

🔹 클래스 주석 예시
/*
* 작성자: 임희섭
* 작성일: 25/11/03
* 클래스 간단 설명 (ex. 플레이어 점멸(Blink)를 위한 컴포넌트)
*/
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TEST_5_6_API UMSBlinkComponent : public UActorComponent
{
    ...
};

📝 커밋 규칙

커밋 메시지는 다음 형식을 사용합니다:

<타입>: <메시지>

타입 목록
NEW: 신규 기능
CHG: 코드 변경
FIX: 문제 수정

📄 라인 엔딩 규칙

모든 파일은 Unix Line Ending(LF) 사용

Windows 환경에서는 다음 명령으로 자동 LF 커밋 설정

git config --global core.autocrlf true

🚀 작업 흐름 요약

develop에서 기능 브랜치 생성

feature/<기능명>에서 개발

커밋 규칙 준수

필요 시 develop에 변경 내용 반영

안정 버전은 main에서 관리