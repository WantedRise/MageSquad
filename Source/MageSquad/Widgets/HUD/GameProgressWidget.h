// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameProgressWidget.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UGameProgressWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

public:
	UPROPERTY(meta=(BindWidget))
	class UProgressEventMarkerWidget* WBP_ProgressIcon_Start;
	UPROPERTY(meta = (BindWidget))
	class UProgressEventMarkerWidget* WBP_ProgressIcon_Event_1;
	UPROPERTY(meta = (BindWidget))
	class UProgressEventMarkerWidget* WBP_ProgressIcon_Event_2;
	UPROPERTY(meta = (BindWidget))
	class UProgressEventMarkerWidget* WBP_ProgressIcon_Event_3;
	UPROPERTY(meta = (BindWidget))
	class UProgressEventMarkerWidget* WBP_ProgressIcon_End;
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* PB_GameProgress;
};
