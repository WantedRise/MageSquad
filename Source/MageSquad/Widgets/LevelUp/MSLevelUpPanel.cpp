// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/LevelUp/MSLevelUpPanel.h"

#include "Components/TextBlock.h"
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
	if (CountdownTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(CountdownTickerHandle);
		CountdownTickerHandle.Reset();
	}
	
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

	// ✅ UI는 닫지 말고 입력만 막기(대기 상태)
	SetIsEnabled(false);
	
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

void UMSLevelUpPanel::StartCountdown(float RemainingSeconds)
{
	if (!RemainingTimeText)
		return;

	// 리얼타임 기반 (Pause 영향 없음)
	CountdownEndPlatformSeconds = FPlatformTime::Seconds() + FMath::Max(0.f, RemainingSeconds);

	// 기존 티커 제거
	if (CountdownTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(CountdownTickerHandle);
		CountdownTickerHandle.Reset();
	}

	CountdownTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UMSLevelUpPanel::TickCountdown),
		0.05f
	);
}

bool UMSLevelUpPanel::TickCountdown(float DeltaTime)
{
	if (!RemainingTimeText)
		return true;

	const double Now = FPlatformTime::Seconds();
	const double Remain = CountdownEndPlatformSeconds - Now;

	const int32 Sec = FMath::Max(0, (int32)FMath::CeilToInt((float)Remain));
	RemainingTimeText->SetText(FText::AsNumber(Sec));

	// 0되면 표시만 유지, 티커는 끊어도 됨
	if (Sec <= 0)
	{
		FTSTicker::GetCoreTicker().RemoveTicker(CountdownTickerHandle);
		CountdownTickerHandle.Reset();
		return false;
	}
	return true;
}
