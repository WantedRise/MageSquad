// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MSLobbyGameMode.generated.h"

/*
* 작성자: 이상준
* 작성일: 2025-12-16
* 로비 단계에서의 서버 권한 게임 규칙을 담당하는 GameMode 클래스.
*
* - 플레이어 접속(PostLogin) 시 로비 초기 설정 처리
* - 플레이어 시작 위치(PlayerStart) 결정
* - 플레이어 준비 상태를 종합하여 게임 시작 조건 판단
* - 준비 완료 시 인게임 레벨로 전환
*/
UCLASS()
class MAGESQUAD_API AMSLobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	AMSLobbyGameMode();
	//AMSLobbyPlayerState에 호스트 여부와 플레이어 닉네임 지정
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void RestartPlayer(AController* NewPlayer) override;
	//로비에서 각 플레이어의 시작 위치를 결정
	AActor* ChoosePlayerStart_Implementation(class AController* Player) override;
public:
	//준비 카운트다운 완료 시 인게임 레벨로 전환
	void HandleReadyCountdownFinished();
	//플레이어 준비 상태 변경 시 호출되어 전체 준비 상태를 재계산
	void HandlePlayerReadyStateChanged();

};
