// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MSLobbyPlayerEntryWidget.generated.h"


/*
* 작성자: 이상준
* 작성일: 2025-12-16
* AMSLobbyCharacter에 부착되는 로비 플레이어 엔트리 UI 위젯.
* PlayerState의 동기화된 데이터를 기반으로 준비 상태, 닉네임, 호스트 여부를 표시한다.
*/
UCLASS()
class MAGESQUAD_API UMSLobbyPlayerEntryWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void ShowHostIcon();
	void SetTextNickName(FString InNickName);
	void SetTextReadyStatus(bool bReady);
public:
	UPROPERTY(meta = (BindWidget))
	class UImage* Image_HostIcon;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_NickName;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_ReadyStatus;
};
