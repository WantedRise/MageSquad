// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFlow/Mission/MSMissionFindWithinTimeScript.h"
#include "Components/MSMissionComponent.h"
#include "Actors/Mission/MSMissionFindTargetActor.h"
#include "System/MSEnemySpawnSubsystem.h" 
#include "Actors/TileMap/MSSpawnTileMap.h"
#include <System/MSMissionDataSubsystem.h>

void UMSMissionFindWithinTimeScript::Initialize(UWorld* World)
{
	if (!World)
		return;

	UMSMissionDataSubsystem* MissionDataSubsystem =
		World->GetGameInstance()->GetSubsystem<UMSMissionDataSubsystem>();
	if (!MissionDataSubsystem)
		return;

	const FMSMissionRow* MissionData = MissionDataSubsystem->Find(3);
	if (!MissionData)
		return;

	CurrentFindCount = 0;
	
	SetTargetInfo(OwnerMissionComponent->GetCurrentMissionData().FindTargetActorClass, OwnerMissionComponent->GetCurrentMissionData().FindTargetCount);

	SpawnTargets(World);

	if (OwnerMissionComponent.IsValid())
	{
		OwnerMissionComponent->UpdateMission();
	}
}

void UMSMissionFindWithinTimeScript::Deinitialize()
{
	for (TWeakObjectPtr<AMSMissionFindTargetActor>& Target : SpawnedTargets)
	{
		if (Target.IsValid())
		{
			Target->OnTargetCollected.RemoveAll(this);
			Target->Destroy();
		}
	}
	SpawnedTargets.Empty();
}

void UMSMissionFindWithinTimeScript::SetTargetInfo(TSubclassOf<AActor> InTargetClass, int32 InCount)
{
	UE_LOG(LogTemp,Log,TEXT("SetTargetInfo %d"),InCount)
	TargetActorClass = InTargetClass;
	RequiredFindCount = InCount;
}

void UMSMissionFindWithinTimeScript::SpawnTargets(UWorld* World)
{
	if (!World || !TargetActorClass)
		return;

	UMSEnemySpawnSubsystem* SpawnSubsystem = World->GetSubsystem<UMSEnemySpawnSubsystem>();
	if (SpawnSubsystem)
	{
		
		if (AMSSpawnTileMap* TileMap = SpawnSubsystem->GetSpawnTileMap())
		{
			// 모든 타일들 가져오기
			TArray<FMSSpawnTile> InvisibleTiles = TileMap->GetAllSpawnableTiles();
			TArray<int32> UniqueIndices = GetRandomIndicesLarge(InvisibleTiles.Num()-1, RequiredFindCount);

			for (int32 SpawnTileIndex : UniqueIndices)
			{
				FVector SpawnLocation = InvisibleTiles[SpawnTileIndex].Location;
				AMSMissionFindTargetActor* Target = World->SpawnActor<AMSMissionFindTargetActor>(
					TargetActorClass,
					SpawnLocation,
					FRotator::ZeroRotator
				);

				if (Target)
				{
					Target->OnTargetCollected.AddUObject(this,&UMSMissionFindWithinTimeScript::NotifyTargetFound);
					SpawnedTargets.Add(TWeakObjectPtr<AMSMissionFindTargetActor>(Target));
				}
			}
		}
	}
}

TArray<int32> UMSMissionFindWithinTimeScript::GetRandomIndicesLarge(int32 MaxIndex, int32 Count)
{
    TSet<int32> UniqueIndices;
    
    while (UniqueIndices.Num() < Count)
    {
        int32 RandomIdx = FMath::RandRange(0, MaxIndex - 1);
        UniqueIndices.Add(RandomIdx); // TSet은 중복을 자동으로 무시함
    }

    return UniqueIndices.Array();
}

void UMSMissionFindWithinTimeScript::NotifyTargetFound()
{
	if (!OwnerMissionComponent.IsValid())
		return;

	++CurrentFindCount;

	OwnerMissionComponent->UpdateMission();
}

float UMSMissionFindWithinTimeScript::GetProgress() const
{
	if (RequiredFindCount <= 0)
		return 1.f;

	return FMath::Clamp(
		static_cast<float>(CurrentFindCount) / RequiredFindCount,
		0.f,
		1.f
	);
}

bool UMSMissionFindWithinTimeScript::IsCompleted() const
{
	return CurrentFindCount >= RequiredFindCount;
}
