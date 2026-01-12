// Fill out your copyright notice in the Description page of Project Settings.


#include "System/MSMissionDataSubsystem.h"

void UMSMissionDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    LoadMissionTable();
}

void UMSMissionDataSubsystem::LoadMissionTable()
{
    MissionTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/Mission/DT/DT_MissionData"));

    if (!MissionTable)
    {
        UE_LOG(LogTemp, Error, TEXT("MissionTable is null"));
        return;
    }

    MissionData.Empty();

    for (const auto& Pair : MissionTable->GetRowMap())
    {
        const FMSMissionRow* Row = reinterpret_cast<FMSMissionRow*>(Pair.Value);
        if (Row)
        {
            MissionData.Add(Row->MissionID, *Row);
            UE_LOG(LogTemp, Log, TEXT("[MissionDataSubsystem] Add mission row."));
        }
        else
        {
            UE_LOG(LogTemp, Log,TEXT("[MissionDataSubsystem] Missing load mission row."));
        }
    }
}

const FMSMissionRow* UMSMissionDataSubsystem::Find(int32 MissionID) const
{
    return MissionData.Find(MissionID);
}

