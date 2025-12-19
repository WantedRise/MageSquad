// Fill out your copyright notice in the Description page of Project Settings.


#include "MSGA_SkillBase.h"

UMSGA_SkillBase::UMSGA_SkillBase()
{
	// Ability 인스턴싱/네트워크 정책
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UMSGA_SkillBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// 데이터테이블 불러오기
	if (!SkillDataTable)
	{
		static const TCHAR* SkillDTPath = TEXT("/Game/Data/Skill/DT_SkillData.DT_SkillData");

		UDataTable* LoadedTable = LoadObject<UDataTable>(nullptr, SkillDTPath);
		if (!LoadedTable)
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] Failed to load SkillDataTable from %s"),
				*GetName(), SkillDTPath);

			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}
		SkillDataTable = LoadedTable;
	}
	
	// 스킬 레벨 가져오기
	const int32 AbilityLevel = GetAbilityLevel(Handle, ActorInfo);
	
	bool bFound = false;
	for (const auto& Pair : SkillDataTable->GetRowMap())
	{
		const FMSSkillDataRow* Row =
			reinterpret_cast<const FMSSkillDataRow*>(Pair.Value);

		if (!Row)
			continue;

		if (Row->SkillID == SkillID &&
			Row->SkillLevel == AbilityLevel)
		{
			SkillDataRow = *Row;
			bFound = true;
			break;
		}
	}
	
	if (!bFound)
	{
		UE_LOG(LogTemp, Error,
			TEXT("[%s] SkillData not found (SkillID=%d, Level=%d)"),
			*GetName(), SkillID, AbilityLevel);

		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
}
