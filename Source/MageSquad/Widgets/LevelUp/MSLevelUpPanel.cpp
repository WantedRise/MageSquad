// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/LevelUp/MSLevelUpPanel.h"

#include "Player/MSPlayerController.h"

void UMSLevelUpPanel::InitPanel(int32 InSessionId, const TArray<FMSLevelUpChoicePair>& InChoices)
{
	SessionId = InSessionId;
	bHasPicked = false;

	// 필수 요소 검증
	if (!ChoiceContainer || !ChoiceWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[LevelUpPanel] InitPanel failed (Container or WidgetClass is null)"));
		return;
	}

	// 기존 선택지 제거
	ChoiceContainer->ClearChildren();

	// UI 입력 모드로 전환
	ApplyUIInputMode();

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return;
	}

	// 선택지 카드 생성
	for (const FMSLevelUpChoicePair& Choice : InChoices)
	{
		UMSLevelUpChoice* ChoiceWidget = CreateWidget<UMSLevelUpChoice>(PC, ChoiceWidgetClass);
		if (!ChoiceWidget)
		{
			continue;
		}

		ChoiceWidget->InitChoice(SessionId, Choice);
		ChoiceWidget->OnChoiceClicked.AddDynamic(this, &UMSLevelUpPanel::HandleChoiceClicked);

		ChoiceContainer->AddChild(ChoiceWidget);
	}
}

void UMSLevelUpPanel::ClosePanel()
{
	RestoreGameInputMode();
	RemoveFromParent();
}

void UMSLevelUpPanel::HandleChoiceClicked(const FMSLevelUpChoicePair& Picked)
{
	// 중복 클릭 방지
	if (bHasPicked)
	{
		return;
	}
	bHasPicked = true;

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		ClosePanel();
		return;
	}

	// 서버와의 통신은 PlayerController가 담당
	if (AMSPlayerController* MSPC = Cast<AMSPlayerController>(PC))
	{
		// Server RPC
		MSPC->Server_SelectSkillLevelUpChoice(SessionId, Picked);
	}

	ClosePanel();
}

void UMSLevelUpPanel::ApplyUIInputMode()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return;
	}

	// 기존 커서 상태 저장
	bPrevShowMouseCursor = PC->bShowMouseCursor;

	FInputModeUIOnly Mode;
	Mode.SetWidgetToFocus(TakeWidget());
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	PC->SetInputMode(Mode);
	PC->bShowMouseCursor = true;
}

void UMSLevelUpPanel::RestoreGameInputMode()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return;
	}

	FInputModeGameOnly Mode;
	PC->SetInputMode(Mode);
	PC->bShowMouseCursor = bPrevShowMouseCursor;
}
