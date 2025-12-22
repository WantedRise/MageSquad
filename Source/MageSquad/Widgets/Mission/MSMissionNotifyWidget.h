// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MSMissionNotifyWidget.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UMSMissionNotifyWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetMissionMessage(FText InMessage);
	void PlayNotify(FText InMessage);
private:
	UFUNCTION()
	void OnNotifyFinished();
public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_MissionMessage;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	class UWidgetAnimation* Anim_ShowNotify;
};
