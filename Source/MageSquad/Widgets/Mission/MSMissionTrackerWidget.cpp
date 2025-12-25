// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Mission/MSMissionTrackerWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Animation/WidgetAnimation.h"
#include "GameStates/MSGameState.h"

void UMSMissionTrackerWidget::SetMissionTitle(FText InTitle)
{
    if (!Text_MissionTitle) return;

	Text_MissionTitle->SetText(InTitle);
}

void UMSMissionTrackerWidget::SetMissionMessage(FText InMessage)
{
    if (!Text_MissionMessage) return;

	Text_MissionMessage->SetText(InMessage);
}

void UMSMissionTrackerWidget::StartMissionTimer(AMSGameState* InGameState, float InEndTime)
{
    MissionEndTime = InEndTime - 0.5f;
    GameState = InGameState;

    UpdateRemainingTime();

    GetWorld()->GetTimerManager().SetTimer(
        UITimerHandle,
        this,
        &UMSMissionTrackerWidget::UpdateRemainingTime,
        1.0f,
        true
    );
}

void UMSMissionTrackerWidget::UpdateRemainingTime()
{
    const float ServerTime = GameState->GetServerTime();
    const float RemainingSeconds = FMath::Max(0.f, MissionEndTime - ServerTime);

    if (RemainingSeconds < 0.f)
    {
        GetWorld()->GetTimerManager().ClearTimer(UITimerHandle);
        Text_Timer->SetText(FText::FromString(TEXT("00:00")));
        SetVisibility(ESlateVisibility::Collapsed);
        return;
    }

    int32 TotalSeconds = FMath::Max(0, FMath::FloorToInt(RemainingSeconds));

    FTimespan Timespan = FTimespan::FromSeconds(TotalSeconds);
    // 00:00 형식으로 출력
    FText FormattedTime = FText::AsTimespan(Timespan);

    Text_Timer->SetText(FormattedTime);
}

void UMSMissionTrackerWidget::SetTargetHpProgress(float InNormalized)
{
    Progress_TargetHp->SetPercent(InNormalized);
}

void UMSMissionTrackerWidget::StopMissionTimer()
{
    GetWorld()->GetTimerManager().ClearTimer(UITimerHandle);
}
