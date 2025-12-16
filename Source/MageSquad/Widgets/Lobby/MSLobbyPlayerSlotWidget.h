// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MSLobbyPlayerSlotWidget.generated.h"

/*
* 작성자: 이상준
* 작성일: 25/12/16
* 로비 플레이어 슬롯에 부착되어 초대 버튼을 통해 Steam 친구 초대 화면을 표시하는 역할
*/
UCLASS()
class MAGESQUAD_API UMSLobbyPlayerSlotWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
public:
	UFUNCTION()
	void ShowFriendList();
		
public:
	UPROPERTY(meta = (BindWidget))
	class UButton* Button_Invite;
};
