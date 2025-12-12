// Fill out your copyright notice in the Description page of Project Settings.


#include "MSGA_AutoActiveSkillBase.h"

UMSGA_AutoActiveSkillBase::UMSGA_AutoActiveSkillBase()
{
	// Ability 인스턴싱/네트워크 정책
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UMSGA_AutoActiveSkillBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// 데이터테이블 불러오기
	if (!SkillDataTable)
	{
		static const TCHAR* SkillDTPath = TEXT("/Game/Data/Skill/DA_SkillData.DA_SkillData");

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
}
