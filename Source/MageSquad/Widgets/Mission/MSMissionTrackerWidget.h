// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MSMissionTrackerWidget.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UMSMissionTrackerWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetMissionTitle(FText InTitle);
	void SetMissionMessage(FText Desc);
	void UpdateRemainingTime(float RemainingSeconds);
	void StartMissionTimer(class AMSGameState* InGameState,float InEndTime);
	void UpdateRemainingTime();
	void SetTargetHpProgress(float InNormalized);
public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_Timer;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_MissionTitle;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_MissionMessage;
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* Progress_TargetHp;

	FTimerHandle UITimerHandle;
	float MissionEndTime;
	AMSGameState* GameState;
};
