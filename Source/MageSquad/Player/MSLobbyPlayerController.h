// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MSLobbyPlayerController.generated.h"

struct FMSCharacterData;

/*
* 작성자: 이상준
* 작성일: 2025-12-16
* 로비 단계에서 LobbyMainWidget 생성을 담당하는 PlayerController.
*
* - 로컬 플레이어의 Ready 입력을 처리하여 서버에 전달
* - LobbyMainWidget(Ready 버튼, 카운트다운 등) 생성 및 관리
* - 로비 전용 카메라로 시점 전환 처리
*/
UCLASS()
class MAGESQUAD_API AMSLobbyPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AMSLobbyPlayerController();

public:
	
	//로컬 플레이어의 준비 상태 변경 요청을 서버에 전달한다.
	//실제 Ready 상태 변경 및 판단은 서버(GameMode/PlayerState)에서 처리된다.
	UFUNCTION(Server, Reliable)
	void ServerRequestSetReady(bool bNewReady);
	UFUNCTION(Server, Reliable)
	void Server_SelectCharacter(TSubclassOf<AMSPlayerCharacter> SelectedClass);
protected:
	//로비 UI 생성과 카메라 설정
	void BeginPlay() override;
	virtual void OnRep_PlayerState() override;
protected:
	//로비 메인 UI 위젯을 생성하고 화면에 표시
	void CreateLobbyUI();
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<class UUserWidget> LobbyMainWidgetClass;
	UPROPERTY()
	class UMSLobbyMainWidget* LobbyMainWidget;
};
