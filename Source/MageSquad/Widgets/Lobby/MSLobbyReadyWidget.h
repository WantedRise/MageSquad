// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/LobbyReadyTypes.h"
#include "MSLobbyReadyWidget.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UMSLobbyReadyWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;
public:
	UFUNCTION()
	void OnReadyTimeChanged(int32 RemainingSeconds);
	UFUNCTION()
	void ApplyReadyStateUI(ELobbyReadyPhase NewLobbyReadyPhase);
protected:
	UFUNCTION()
	void OnReadyButtonClicked();

public:
	UPROPERTY(meta = (BindWidget))
	class UButton* Button_Ready;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_Ready_Default;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_Ready_Selected;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_Ready_Second;
private:
	bool  bReady;
};
