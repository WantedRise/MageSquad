// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFlow/Mission/MSMissionKillTargetScript.h"
#include "GameFramework/Actor.h"
#include "System/MSEnemySpawnSubsystem.h"
#include "Enemy/MSBaseEnemy.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AttributeSets/MSEnemyAttributeSet.h"
#include "Components/MSMissionComponent.h"
#include "Utils/MSUtils.h"
#include "MSGameplayTags.h"
#include "DataStructs/MSMissionProgressUIData.h"

// UMSMission_EliteKillScript.cpp
void UMSMissionKillTargetScript::Initialize(UWorld* World)
{
	Progress = 1.f;

	SpawnElite(World);

	if (!EliteMonster.IsValid())
		return;

	UAbilitySystemComponent* ASC = EliteMonster->GetAbilitySystemComponent();
	if (!ASC)
		return;

	const UMSEnemyAttributeSet* AttributeSet = EliteMonster->GetAttributeSet();
	if (!AttributeSet)
		return;


	// GAS Attribute 변경 델리게이트 바인딩
	MaxHp = AttributeSet->GetMaxHealth();
	CurrentHp = MaxHp;
	ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetCurrentHealthAttribute()).AddUObject(
		this, &UMSMissionKillTargetScript::OnEliteHPChanged);

	if (OwnerMissionComponent.IsValid())
	{
		OwnerMissionComponent->UpdateMission();
	}
}

void UMSMissionKillTargetScript::Deinitialize()
{
	EliteMonster = nullptr;
}

void UMSMissionKillTargetScript::SpawnElite(UWorld* World)
{
	if (UMSEnemySpawnSubsystem* SpawnSystem = UMSEnemySpawnSubsystem::Get(World))
	{
		EliteMonster = SpawnSystem->SpawnMonsterByID(MSUtils::ENEMY_BOSS_SEVAROG, FVector(0, 0, 0));
		UE_LOG(LogTemp, Error, TEXT("%s"),
			EliteMonster != nullptr ? TEXT("EliteMonster Spawn") : TEXT("EliteMonster Not Spawn"));
	}
}

void UMSMissionKillTargetScript::OnEliteHPChanged(const FOnAttributeChangeData& Data)
{
	CurrentHp = Data.NewValue;

	if (OwnerMissionComponent.IsValid())
	{
		OwnerMissionComponent->UpdateMission();
	}
}

void UMSMissionKillTargetScript::GetProgress(FMSMissionProgressUIData& OutData) const
{
	OutData.MissionType = EMissionType::Boss;
	OutData.CurrentHp = CurrentHp;
	OutData.MaxHp = MaxHp;
	OutData.Normalized = FMath::Clamp((CurrentHp / MaxHp), 0.0f, 1.0f);
}

bool UMSMissionKillTargetScript::IsCompleted() const
{
	if (CurrentHp / MaxHp > 0.0f)
		return  false;

	// 2페이지 판정
	if (UAbilitySystemComponent* ASC = EliteMonster->GetAbilitySystemComponent())
	{
		if (!ASC->HasMatchingGameplayTag(MSGameplayTags::Enemy_State_Phase2))
		{
			return false;
		}
	}

	return CurrentHp / MaxHp <= 0.0f;
}

