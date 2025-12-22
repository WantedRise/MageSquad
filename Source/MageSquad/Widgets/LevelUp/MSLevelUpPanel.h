// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Types/MageSquadTypes.h"
#include "Widgets/LevelUp/MSLevelUpChoice.h"
#include "MSLevelUpPanel.generated.h"

/**
 * 작성자 : 박세찬
 * 작성일 : 25/12/22
 * 
 * 스킬 레벨업 선택지(MSLevelUpChoice)를 화면에 띄우는 위젯
 */
UCLASS()
class MAGESQUAD_API UMSLevelUpPanel : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void InitPanel(int32 InSessionId, const TArray<FMSLevelUpChoicePair>& InChoices);

	UFUNCTION(BlueprintCallable)
	void ClosePanel();

protected:
	UFUNCTION()
	void HandleChoiceClicked(const FMSLevelUpChoicePair& Picked);

	void ApplyUIInputMode();
	void RestoreGameInputMode();

protected:
	UPROPERTY(meta = (BindWidget))
	UPanelWidget* ChoiceContainer = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Button_Close = nullptr;

	UPROPERTY(EditDefaultsOnly, Category="LevelUp")
	TSubclassOf<UMSLevelUpChoice> ChoiceWidgetClass;

private:
	int32 SessionId = 0;
	bool bHasPicked = false;

	bool bPrevShowMouseCursor = false;
};
