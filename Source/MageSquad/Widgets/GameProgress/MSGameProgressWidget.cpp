// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GameProgress/MSGameProgressWidget.h"
#include "Components/SizeBox.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"
#include "GameStates/MSGameState.h"
#include "MSMissionEventMarker.h"

void UMSGameProgressWidget::NativeConstruct()
{
    Super::NativeConstruct();


    // Geometry 계산을 위해 강제 Prepass
    ForceLayoutPrepass();
    UE_LOG(LogTemp, Warning, TEXT("UGameProgressWidget NativeConstruct"));

    TryInitializGameProgress();
}

void UMSGameProgressWidget::AddProgressEventMarker(int32 MissionID, float Percent)
{
    if (!ProgressEventMarkerClass || !MissionEventLayer || !PB_GameProgress)
    {
        return;
    }
    UE_LOG(LogTemp, Error, TEXT("AddProgressEventMarker success"));
    Percent = FMath::Clamp(Percent, 0.f, 1.f);

    // 이벤트 마커 생성
    UMSMissionEventMarker* MarkerWidget = CreateWidget<UMSMissionEventMarker>(GetWorld(), ProgressEventMarkerClass);

    if (!MarkerWidget)
    {
        UE_LOG(LogTemp, Error, TEXT("AddProgressEventMarker MarkerWidget nullptr"));
        return;
    }
    MarkerWidget->SetMissionID(MissionID);
    MissionEventLayer->AddChild(MarkerWidget);

    // SizeBox 실제 Width 가져오기
    const float TrackWidth = SizeBox_Track->GetCachedGeometry().GetLocalSize().X;

    const float XPos = (TrackWidth * Percent);
    UE_LOG(LogTemp, Error, TEXT("AddProgressEventMarker %f"), XPos);
    if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MarkerWidget->Slot))
    {
        CanvasSlot->SetSize(FVector2D(100.0f,30.0f));

        // 좌측 기준 (0,0)
        CanvasSlot->SetAnchors(FAnchors(0.f, 0.f));

        // 마커 중앙을 기준으로 위치
        CanvasSlot->SetAlignment(FVector2D(0.0f, -1.0f));

        // 가운데 위치하기 위해 이미지 절반 길이만큼 빼줌
        CanvasSlot->SetPosition(FVector2D(XPos - 32.f, 0.f));
    }
}


void UMSGameProgressWidget::TryInitializGameProgress()
{
    AMSGameState* GS = GetWorld()->GetGameState<AMSGameState>();
    if (GS)
    {
        GS->OnMissionChanged.AddUObject(this, &UMSGameProgressWidget::HandleMissionChanged);
        GS->OnProgressUpdated.AddUObject(this, &UMSGameProgressWidget::OnProgressUpdated);
        OnProgressUpdated(GS->GetProgressNormalized());
    }
    else
    {
        // 일정 시간 후 재시도
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UMSGameProgressWidget::TryInitializGameProgress, 0.2f, false);
    }
}

void UMSGameProgressWidget::OnProgressUpdated(float Normalized)
{
    TargetPercent = Normalized;
}

void UMSGameProgressWidget::HandleMissionChanged(int32 MissionID)
{
    UE_LOG(LogTemp, Error, TEXT("HandleMissionChanged"));

    AMSGameState* GS = GetWorld()->GetGameState<AMSGameState>();
    if (GS)
    {
        AddProgressEventMarker(MissionID, CurrentPercent);
    }
}

void UMSGameProgressWidget::NativeTick(
    const FGeometry& MyGeometry,
    float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!PB_GameProgress)
        return;

    CurrentPercent = FMath::FInterpTo(
        CurrentPercent,
        TargetPercent,
        InDeltaTime,
        InterpSpeed
    );

    PB_GameProgress->SetPercent(CurrentPercent);
}