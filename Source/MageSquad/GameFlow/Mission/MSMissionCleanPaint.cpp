// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFlow/Mission/MSMissionCleanPaint.h"
#include "Actors/Mission/MSInkAreaActor.h"
#include "Kismet/GameplayStatics.h"
void UMSMissionCleanPaint::Initialize(UWorld* World)
{
    Progress = 1.f;

    if (!World)
        return;

    // 서버에서만 판정
    if (!World->GetAuthGameMode())
        return;

    BindInkAreas(World);

    // 초기 상태 1회 계산
    OnInkAreaUpdated();
}

void UMSMissionCleanPaint::Deinitialize()
{
    InkAreas.Empty();
}

void UMSMissionCleanPaint::BindInkAreas(UWorld* World)
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(
        World,
        AMSInkAreaActor::StaticClass(),
        FoundActors
    );

    for (AActor* Actor : FoundActors)
    {
        AMSInkAreaActor* Area = Cast<AMSInkAreaActor>(Actor);
        if (!Area)
            continue;

        InkAreas.Add(Area);

        // 🔥 EliteKillScript의 GAS Delegate와 동일한 역할
        //Area->OnInkLogicUpdated.AddUObject(
        //    this,
        //    &UMSMissionCleanPaint::OnInkAreaUpdated
        //);
    }

    UE_LOG(LogTemp, Log, TEXT("UMSMissionCleanPaint: Bound %d InkAreas"), InkAreas.Num());
}

void UMSMissionCleanPaint::OnInkAreaUpdated()
{
    float SumRemainingRatio = 0.f;
    int32 ValidCount = 0;

    for (const TWeakObjectPtr<AMSInkAreaActor>& AreaPtr : InkAreas)
    {
        if (!AreaPtr.IsValid())
            continue;

        // GetCleanRatio() = 0~1 (1 = 깨끗)
        const float CleanRatio = AreaPtr->GetCleanRatio();

        // Progress는 "남은 오염 비율"
        const float RemainingDirtyRatio = 1.f - CleanRatio;

        SumRemainingRatio += RemainingDirtyRatio;
        ++ValidCount;
    }

    if (ValidCount > 0)
    {
        Progress = FMath::Clamp(
            SumRemainingRatio / ValidCount,
            0.f,
            1.f
        );
    }
    else
    {
        Progress = 0.f;
    }

    UE_LOG(LogTemp, Verbose,
        TEXT("UMSMissionCleanPaint::OnInkAreaUpdated Progress = %.3f"),
        Progress
    );
}

float UMSMissionCleanPaint::GetProgress() const
{
    return Progress;
}
