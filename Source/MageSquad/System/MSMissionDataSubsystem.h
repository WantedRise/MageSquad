// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DataStructs/MSGameMissionData.h"
#include "MSMissionDataSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UMSMissionDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	const FMSMissionRow* Find(int32 ID) const;
private:
	void LoadMissionTable();
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Mission Data")
	UDataTable* MissionTable;
	TMap<int32, FMSMissionRow> MissionData;
};
