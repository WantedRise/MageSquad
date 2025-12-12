// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MSLobbyGameMode.generated.h"

/*
* 작성자: 이상준
* 작성일: 25/12/06
* 로비에서 플레이어 시작위치와 게임시작/게임결과를 제어를 위한 클래스
*/
UCLASS()
class MAGESQUAD_API AMSLobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	AMSLobbyGameMode();

	AActor* ChoosePlayerStart_Implementation(class AController* Player) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

};
