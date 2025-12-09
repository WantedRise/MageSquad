// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MSLobbyPlayerSlotWidget.generated.h"

/*
* 작성자: 이상준
* 작성일: 25/12/08
* 로비에서 초대 표시하는 위젯 클래스
*/
UCLASS()
class MAGESQUAD_API UMSLobbyPlayerSlotWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void NativeConstruct() override;

	UFUNCTION()
	void ShowFriendList();
public:
	UPROPERTY(meta = (BindWidget))
	class UButton* Button_Invite;
};
