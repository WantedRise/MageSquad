// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Mission/MSMissionTrackerWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Animation/WidgetAnimation.h"
#include "GameStates/MSGameState.h"
#include "Components/SizeBox.h"
#include "Types/GameMissionTypes.h"

void UMSMissionTrackerWidget::NativeDestruct()
{
    Super::NativeDestruct();

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(UITimerHandle);
    }
}

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
    if (!GameState)
    {
        return;
    }
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

void UMSMissionTrackerWidget::UpdateProgress(const FMSMissionProgressUIData& Data)
{
    switch (Data.MissionType)
    {
    case EMissionType::Boss:
        ShowBossProgress();
        Progress_BossHp->SetPercent(Data.Normalized);
        SetTextBossHp(Data.CurrentHp,Data.MaxHp);
        break;
    case EMissionType::FindTarget:
        ShowFindTargetUI();
        Text_FindTarget->SetText(
            FText::FromString(
                FString::Printf(
                    TEXT("%d / %d"),
                    Data.CurrentCount,
                    Data.TargetCount
                )
            )
        );
        break;
    default:
        ShowDefaultProgress();
        Progress_TargetHp->SetPercent(Data.Normalized);
    }
}

void UMSMissionTrackerWidget::SetBossHpProgress(float InNormalized)
{
    Progress_BossHp->SetPercent(InNormalized);
}
void UMSMissionTrackerWidget::ShowDefaultProgress()
{
    Text_Timer->SetVisibility(ESlateVisibility::Visible);
    SizeBox_Boss->SetVisibility(ESlateVisibility::Collapsed);
    SizeBox_Other->SetVisibility(ESlateVisibility::Visible);
    Text_FindTarget->SetVisibility(ESlateVisibility::Collapsed);
}
void UMSMissionTrackerWidget::ShowBossProgress()
{
    StopMissionTimer();
    Text_Timer->SetVisibility(ESlateVisibility::Collapsed);
    SizeBox_Boss->SetVisibility(ESlateVisibility::Visible);
    SizeBox_Other->SetVisibility(ESlateVisibility::Collapsed);
    Text_FindTarget->SetVisibility(ESlateVisibility::Collapsed);
}
void UMSMissionTrackerWidget::ShowFindTargetUI()
{
    Text_FindTarget->SetVisibility(ESlateVisibility::Visible);
    SizeBox_Boss->SetVisibility(ESlateVisibility::Collapsed);
    SizeBox_Other->SetVisibility(ESlateVisibility::Collapsed);
}


void UMSMissionTrackerWidget::SetTextBossHp(float CurrentHp, float MaxHp)
{
    if (!Text_BossHp) return;

    // 정수형(int32)으로 변환하여 출력
    int32 Current = FMath::FloorToInt(CurrentHp);
    int32 Max = FMath::FloorToInt(MaxHp);

    // {0} / {1} 형태로 포맷 생성
    FText HpText = FText::Format(
        NSLOCTEXT("UI", "BossHpFormat", "{0}/{1}"),
        FText::AsNumber(Current),
        FText::AsNumber(Max)
    );

    Text_BossHp->SetText(HpText);
}

void UMSMissionTrackerWidget::UpdateFindCount(int32 Current, int32 Target)
{
    Text_FindTarget->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), Current, Target)));
}

void UMSMissionTrackerWidget::StopMissionTimer()
{
    GetWorld()->GetTimerManager().ClearTimer(UITimerHandle);
}
