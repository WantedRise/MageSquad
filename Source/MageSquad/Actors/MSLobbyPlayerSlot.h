// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MSLobbyPlayerSlot.generated.h"

/*
* 작성자: 이상준
* 작성일: 25/12/07
* 수정일: 25/12/16
* 로비 씬에 배치되는 플레이어 슬롯용 Actor로, 캐릭터 스폰 기준이 되며
* Steam 친구 초대를 위한 슬롯 UI를 월드 공간에 표시한다.
*
* GameMode의 ChoosePlayerStart 로직에서 선택되어 플레이어 캐릭터의 스폰 위치로 사용
* - 슬롯에 플레이어가 배정되지 않은 경우 초대 UI 표시
* - 플레이어가 배정되면 초대 UI 숨김
*
* 실제 초대 동작은 UMSLobbyPlayerSlotWidget과
* UMSSteamManagerSubsystem을 통해 처리
*/
UCLASS()
class MAGESQUAD_API AMSLobbyPlayerSlot : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMSLobbyPlayerSlot();
	AController* GetController() const { return PlayerController; }
	void SetController(AController* NewPlayer) { PlayerController = NewPlayer; }
	void HiddenInviteWidgetComponent();
	void ShowInviteWidgetComponent();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
protected:
	UPROPERTY(EditAnywhere,meta=(AllowPrivateAccess = "true"))
	class UWidgetComponent* PlaySlotWidgetComponent;
	UPROPERTY()
	class UMSLobbyPlayerSlotWidget* UPlayerSlotWidget;
	UPROPERTY(VisibleAnywhere)
	class AController* PlayerController;

};
