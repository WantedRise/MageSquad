// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFlow/Mission/MSMissionCleanPaint.h"
#include "Actors/Mission/MSInkAreaActor.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Components/MSMissionComponent.h"

UMSMissionCleanPaint::UMSMissionCleanPaint()
{
    static ConstructorHelpers::FClassFinder<AMSInkAreaActor> BP_Class(TEXT("/Game/Blueprints/Actors/Mission/BP_InkArea.BP_InkArea_C"));

    if (BP_Class.Succeeded())
    {
        MissionActorClass = BP_Class.Class;
    }
}
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
    if (NavSystem->ProjectPointToNavigation(FVector(0,0,0), NavLoc, FVector(1000.0f, 1000.0f, 1000.0f)))
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

    InkAreas.Add(SpawnedActor);

    //BindInkAreas(World);
    // 초기 상태 1회 계산
    OnAreaProgressChanged(GetProgress());
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

void UMSMissionCleanPaint::OnAreaProgressChanged(float)
{
    if (OwnerMissionComponent.IsValid())
    {
        OwnerMissionComponent->UpdateMission();
    }
}

float UMSMissionCleanPaint::GetProgress() const
{
    if (InkAreas.Num() == 0)
        return 1.f;

    float Total = 0.f;

    for (const TWeakObjectPtr<AMSInkAreaActor>& Area : InkAreas)
    {
        if (!Area.IsValid())
            continue;

        const float Ratio = Area->GetCleanRatio();
        //보정 

        Total += (Ratio >= AreaCompleteThreshold) ? 1.f : Ratio;
    }

    return Total / InkAreas.Num();
}
