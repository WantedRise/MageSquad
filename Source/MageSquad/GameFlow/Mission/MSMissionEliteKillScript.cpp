// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFlow/Mission/MSMissionEliteKillScript.h"
#include "GameFramework/Actor.h"
#include "System/MSEnemySpawnSubsystem.h"
#include "Enemy/MSBaseEnemy.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AttributeSets/MSEnemyAttributeSet.h"
#include "Components/MSMissionComponent.h"

// UMSMission_EliteKillScript.cpp
void UMSMissionEliteKillScript::Initialize(UWorld* World)
{
    Progress = 0.f;

    SpawnElite(World);
   
    if (!EliteMonster.IsValid())
        return;

    UAbilitySystemComponent* ASC = EliteMonster->GetAbilitySystemComponent();
    if (!ASC)
        return;

    const UMSEnemyAttributeSet* AttributeSet = EliteMonster->GetAttributeSet();
    if (!AttributeSet)
        return;

    UE_LOG(LogTemp, Error, TEXT("UAbilitySystemComponent UMSEnemyAttributeSet"));
    // 🔥 GAS Attribute 변경 델리게이트 바인딩
    MaxHP = AttributeSet->GetMaxHealth();
    ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetCurrentHealthAttribute()).AddUObject(this,&UMSMissionEliteKillScript::OnEliteHPChanged);
   
    UE_LOG(LogTemp, Error, TEXT("UAbilitySystemComponent UMSEnemyAttributeSet %f"), MaxHP);
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
    if (UMSEnemySpawnSubsystem* SpawnSystem = UMSEnemySpawnSubsystem::Get(World))
    {
        
        EliteMonster = SpawnSystem->SpawnMonsterByID(FName(TEXT("Boss_Fey")), FVector(0, 0, 0));
        UE_LOG(LogTemp, Error, TEXT("%s"), EliteMonster!=nullptr ? TEXT("EliteMonster Spawn") : TEXT("EliteMonster Not Spawn"));
    }

    // SpawnActor<AEliteMonster>(...)
}

void UMSMissionEliteKillScript::OnEliteHPChanged(const FOnAttributeChangeData& Data)
{
    UE_LOG(LogTemp, Error, TEXT("OnEliteHPChanged"));
    
    float CurrentHP = Data.NewValue;
    
    Progress = FMath::Clamp((CurrentHP / MaxHP), 0.0f, 1.0f);

    if (OwnerMissionComponent.IsValid())
    {
        OwnerMissionComponent->UpdateMission();
    }
}

float UMSMissionEliteKillScript::GetProgress() const
{
    return Progress;
}
