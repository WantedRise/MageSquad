// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MSLobbyPlayerEntryWidget.generated.h"

/**
 * 
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
