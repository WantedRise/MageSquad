// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/HUD/GameProgressWidget.h"
#include "Components/ProgressBar.h"
#include "GameStates/MSGameState.h"
#include "ProgressEventMarkerWidget.h"

void UGameProgressWidget::NativeConstruct()
{
    Super::NativeConstruct();


    UE_LOG(LogTemp, Warning, TEXT("UGameProgressWidget NativeConstruct"));

    TryInitializGameProgress();
}

void UGameProgressWidget::TryInitializGameProgress()
{
    AMSGameState* GS = GetWorld()->GetGameState<AMSGameState>();
    if (GS)
    {
        GS->OnMissionChanged.AddUObject(this, &UGameProgressWidget::HandleMissionChanged);
        GS->OnProgressUpdated.AddUObject(this, &UGameProgressWidget::OnProgressUpdated);
        OnProgressUpdated(GS->GetProgressNormalized());
    }
    else
    {
        // 일정 시간 후 재시도
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UGameProgressWidget::TryInitializGameProgress, 0.2f, false);
    }
}

void UGameProgressWidget::OnProgressUpdated(float Normalized)
{
    if (PB_GameProgress)
    {
        PB_GameProgress->SetPercent(Normalized);
    }
}

void UGameProgressWidget::HandleMissionChanged(int32 MissionID)
{
    TArray<UProgressEventMarkerWidget*> Slots =
    {
        WBP_ProgressIcon_Event_1,
        WBP_ProgressIcon_Event_2,
        WBP_ProgressIcon_Event_3,
        WBP_ProgressIcon_End
    };

    if (!Slots.IsValidIndex(CurrentSlotIndex))
        return;

    if (Slots[CurrentSlotIndex])
    {
        Slots[CurrentSlotIndex]->SetMissionID(MissionID);
        ++CurrentSlotIndex;
    }
}