// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFlow/Mission/MSMissionCleanPaint.h"
#include "Actors/Mission/MSInkAreaActor.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Components/MSMissionComponent.h"
#include "DataStructs/MSMissionProgressUIData.h"

UMSMissionCleanPaint::UMSMissionCleanPaint()
{
    static ConstructorHelpers::FClassFinder<AMSInkAreaActor> BP_Class(TEXT("/Game/Blueprints/Actors/Mission/BP_InkArea.BP_InkArea_C"));

    if (BP_Class.Succeeded())
    {
        MissionActorClass = BP_Class.Class;
    }
}

//Todo : 랜덤 위치에 N개의 영역을 만들어야함
void UMSMissionCleanPaint::Initialize(UWorld* World)
{
    if (!World)
        return;

    // 서버에서만 판정
    if (!World->GetAuthGameMode())
        return;



    UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    if (!NavSystem)
    {
        return;
    }
    FNavLocation NavLoc;
    FVector OutLocation;
    if (NavSystem->ProjectPointToNavigation(FVector(0,0,0), NavLoc, FVector(2000.0f, 2000.0f, 2000.0f)))
    {
        OutLocation = NavLoc.Location;
        OutLocation.Z = 10.f;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AMSInkAreaActor* SpawnedActor = World->SpawnActor<AMSInkAreaActor>(
        MissionActorClass,
        OutLocation,
        FRotator::ZeroRotator,
        SpawnParams
    );

    SpawnedActor->OnProgressChanged.AddUObject(
        this,
        &UMSMissionCleanPaint::OnAreaProgressChanged
    );

    AMSInkAreaActor* SpawnedActor2 = World->SpawnActor<AMSInkAreaActor>(
        MissionActorClass,
        OutLocation+FVector(0,1000.0f,0),
        FRotator::ZeroRotator,
        SpawnParams
    );

    SpawnedActor2->OnProgressChanged.AddUObject(
        this,
        &UMSMissionCleanPaint::OnAreaProgressChanged
    );

    InkAreas.Add(SpawnedActor);
    InkAreas.Add(SpawnedActor2);
    //BindInkAreas(World);
    // 초기 상태 1회 계산
    OnAreaProgressChanged(0.0f);
}

void UMSMissionCleanPaint::Deinitialize()
{
    for (TWeakObjectPtr<class AMSInkAreaActor> Area : InkAreas)
    {
        if (Area.IsValid())
        {
            Area->OnProgressChanged.RemoveAll(this);
            Area->Destroy();
        }
    }
    InkAreas.Empty();

    Super::Deinitialize();
}

void UMSMissionCleanPaint::OnAreaProgressChanged(float /*AreaRatio*/)
{
    // Percent 재계산
    RecalculateCurrentPercent();

    // 미션 시스템에 변경 통지
    if (OwnerMissionComponent.IsValid())
    {
        OwnerMissionComponent->UpdateMission();
    }
}

void UMSMissionCleanPaint::RecalculateCurrentPercent()
{
    if (InkAreas.Num() == 0)
    {
        CurrentPercent = 1.0f;
        return;
    }

    float Total = 0.f;
    int32 ValidCount = 0;

    for (const TWeakObjectPtr<AMSInkAreaActor>& Area : InkAreas)
    {
        if (!Area.IsValid())
            continue;

        Total += Area->GetCleanRatio(); // 0~1
        ++ValidCount;
    }
    const float RawPercent = (ValidCount > 0) ? (Total / ValidCount) : TargetPercent;
    CurrentPercent = FMath::Clamp(RawPercent / TargetPercent, 0.f, 1.f);
}

void UMSMissionCleanPaint::GetProgress(FMSMissionProgressUIData& OutData) const
{
    OutData.MissionType = EMissionType::ClearPaint;
    OutData.Normalized = CurrentPercent;
}

bool UMSMissionCleanPaint::IsCompleted() const
{
    if (InkAreas.Num() == 0)
    {
        return true;
    }

    return CurrentPercent >= 1.0f;
}
