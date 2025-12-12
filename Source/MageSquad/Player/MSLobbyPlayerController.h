// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MSLobbyPlayerController.generated.h"



/*
* 작성자: 이상준
* 작성일: 25/12/07
* 로비에서의 플레이어 컨트롤러
* 카메라 시점 제어
*/
UCLASS()
class MAGESQUAD_API AMSLobbyPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AMSLobbyPlayerController();

protected:
	void BeginPlay() override;
	void OnPossess(class APawn* NewPawn) override;

private:
	FTimerHandle CameraTimerHandle;
};
