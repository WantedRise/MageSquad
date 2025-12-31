// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GameProgress/MSGameProgressWidget.h"
#include "Components/SizeBox.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"
#include "GameStates/MSGameState.h"
#include "MSMissionEventMarker.h"
#include "Blueprint/SlateBlueprintLibrary.h"

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

    // SizeBox 실제 Width 
    FGeometry Geometry = SizeBox_Track->GetCachedGeometry();
    const float TrackWidth = Geometry.GetLocalSize().X;
    const float XPos = (TrackWidth * Percent);
    // MarkerWidget이 배치될 부모(CanvasPanel 등)의 Geometry
    FGeometry ParentGeometry = MarkerWidget->GetParent()->GetCachedGeometry();

    // SizeBox의 특정 로컬 지점(XPos)을 '절대 좌표(Absolute)'로 변환
    // TrackWidth * Percent 로 계산된 XPos를 사용
    FVector2D LocalPoint(XPos, 0.0f);
    FVector2D AbsolutePoint = Geometry.LocalToAbsolute(LocalPoint);

    // 이 '절대 좌표'를 부모 위젯의 '로컬 좌표'로 다시 변환
    // 이 과정에서 DPI 스케일과 창모드/전체화면 오차가 자동으로 상쇄
    FVector2D FinalLocalPos = ParentGeometry.AbsoluteToLocal(AbsolutePoint);

    // 슬롯 설정
    if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MarkerWidget->Slot))
    {
        CanvasSlot->SetSize(FVector2D(100.0f, 30.0f));

        // 부모의 왼쪽 위(0,0)를 기준으로 잡습니다.
        CanvasSlot->SetAnchors(FAnchors(0.f, 0.f));
        CanvasSlot->SetAlignment(FVector2D(0.0f, -1.0f));

        // 계산된 로컬 좌표를 적용합니다.
        CanvasSlot->SetPosition(FinalLocalPos);
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