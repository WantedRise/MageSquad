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

public:
	UFUNCTION(Server, Reliable)
	void ServerRequestSetReady(bool bNewReady);
protected:
	void BeginPlay() override;
	void InitPlayerState() override;
protected:
	void CreateLobbyUI();
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<class UUserWidget> LobbyMainWidgetClass;
	UPROPERTY()
	class UMSLobbyMainWidget* LobbyMainWidget;
};
