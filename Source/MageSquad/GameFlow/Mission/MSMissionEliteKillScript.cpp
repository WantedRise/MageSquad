// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFlow/Mission/MSMissionEliteKillScript.h"
#include "GameFramework/Actor.h"

// UMSMission_EliteKillScript.cpp
void UMSMissionEliteKillScript::Initialize(UWorld* World)
{
    Progress = 0.f;
    SpawnElite(World);
   
    if (EliteMonster.IsValid())
    {
        //InEliteMonster->OnHPChanged.AddUObject(this,&UMSMission_EliteKillScript::OnEliteHPChanged);
    }
    
}

void UMSMissionEliteKillScript::Deinitialize()
{
    if (EliteMonster.IsValid())
    {
        //EliteMonster->OnHPChanged.RemoveAll(this);
    }

    EliteMonster = nullptr;
}

void UMSMissionEliteKillScript::SpawnElite(UWorld* World)
{
    // SpawnActor<AEliteMonster>(...)
}

void UMSMissionEliteKillScript::OnEliteHPChanged(float CurrentHP,float MaxHP)
{
    Progress = 1.f - (CurrentHP / MaxHP);
}

float UMSMissionEliteKillScript::GetProgress() const
{
    return Progress;
}
