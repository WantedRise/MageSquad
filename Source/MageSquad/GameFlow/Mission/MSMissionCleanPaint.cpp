// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFlow/Mission/MSMissionCleanPaint.h"
#include "Actors/Mission/MSInkAreaActor.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Components/MSMissionComponent.h"
#include "DataStructs/MSMissionProgressUIData.h"
#include "DataStructs/MSMissionCleanData.h"
#include "System/MSEnemySpawnSubsystem.h" 
#include "Actors/TileMap/MSSpawnTileMap.h"
#include "GameStates/MSGameState.h"

UMSMissionCleanPaint::UMSMissionCleanPaint()
{
    static ConstructorHelpers::FClassFinder<AMSInkAreaActor> BP_Class(TEXT("/Game/Blueprints/Actors/Mission/BP_InkArea.BP_InkArea_C"));

    if (BP_Class.Succeeded())
    {
        MissionActorClass = BP_Class.Class;
    }

    //정화미션 데이터 가져오기
    static ConstructorHelpers::FObjectFinder<UDataTable> DataTableAsset(TEXT("/Game/Data/Mission/DT/DT_MissionCleanData.DT_MissionCleanData"));
    if (DataTableAsset.Succeeded())
    {
        CleanMissionDataTable = DataTableAsset.Object;
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
    
    //플레이인원수에 해당하는 정화미션 데이터를 가져오기
    if (AMSGameState* GS = World->GetGameState<AMSGameState>())
    {
        if (CleanMissionDataTable)
        {
            FName RowName = FName(*FString::FromInt(GS->GetActivePlayerCount()));
            const FString Context(TEXT("CleanMissionData"));
            CleanData = CleanMissionDataTable->FindRow<FMSMissionCleanData>(
                RowName,
                TEXT("CleanMission")
            );
        }
    }
    
    if (!CleanData)
    {
        UE_LOG(LogTemp, Warning, TEXT("CleanMissionData not found"));
        return;
    }

    SetTargetInfo(OwnerMissionComponent->GetCurrentMissionData().FindTargetActorClass);
    SpawnTargets(World);

    // 초기 상태 1회 계산
    OnAreaProgressChanged(0.0f);
}

void UMSMissionCleanPaint::SetTargetInfo(TSubclassOf<AActor> InTargetClass)
{
    TargetActorClass = InTargetClass;
}

FVector UMSMissionCleanPaint::CalculateAreaScaleFromSize(float SizeCM)
{
    constexpr float BasePlaneSizeCM = 100.f; // Plane 실제 크기
    const float ScaleXY = SizeCM / BasePlaneSizeCM;

    return FVector(ScaleXY, ScaleXY, 1.f);
}

void UMSMissionCleanPaint::SpawnTargets(UWorld* World)
{
    if (!World || !TargetActorClass || !CleanData)
        return;
    UE_LOG(LogTemp, Log, TEXT("UMSMissionCleanPaint %d,%f"), CleanData->AreaCount, CleanData->AreaSize);
    UMSEnemySpawnSubsystem* SpawnSubsystem = World->GetSubsystem<UMSEnemySpawnSubsystem>();
    if (SpawnSubsystem)
    {

        if (AMSSpawnTileMap* TileMap = SpawnSubsystem->GetSpawnTileMap())
        {
            // 모든 타일들 가져오기
            TArray<FMSSpawnTile> InvisibleTiles = TileMap->GetAllSpawnableTiles();
            TArray<int32> UniqueIndices = GetRandomIndicesLarge(InvisibleTiles.Num() - 1, CleanData->AreaCount);

            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            for (int32 SpawnTileIndex : UniqueIndices)
            {
                FVector SpawnLocation = InvisibleTiles[SpawnTileIndex].Location;
                SpawnLocation.Z += 10.0f;
                AMSInkAreaActor* Target = World->SpawnActor<AMSInkAreaActor>(
                    TargetActorClass,
                    FTransform(FRotator::ZeroRotator, SpawnLocation, CalculateAreaScaleFromSize(CleanData->AreaSize)),
                    SpawnParams
                );

                if (Target)
                {
                    Target->OnProgressChanged.AddUObject(
                        this,
                        &UMSMissionCleanPaint::OnAreaProgressChanged
                    );

                    InkAreas.Add(TWeakObjectPtr<AMSInkAreaActor>(Target));
                }
            }
        }
    }
}

TArray<int32> UMSMissionCleanPaint::GetRandomIndicesLarge(int32 MaxIndex, int32 Count)
{
    TSet<int32> UniqueIndices;

    while (UniqueIndices.Num() < Count)
    {
        int32 RandomIdx = FMath::RandRange(0, MaxIndex - 1);
        UniqueIndices.Add(RandomIdx); // TSet은 중복을 자동으로 무시함
    }

    return UniqueIndices.Array();
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
