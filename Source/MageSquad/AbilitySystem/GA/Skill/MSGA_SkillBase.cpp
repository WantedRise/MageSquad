// Fill out your copyright notice in the Description page of Project Settings.


#include "MSGA_SkillBase.h"

#include "Player/MSPlayerState.h"

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
	const AMSPlayerState* PS = Cast<AMSPlayerState>(ActorInfo ? ActorInfo->OwnerActor.Get() : nullptr);
	if (!PS)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] PlayerState(OwnerActor) is NULL"), *GetName());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FMSSkillList* Skill = PS->GetOwnedSkillByID(SkillID);
	if (!Skill)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] OwnedSkill not found (SkillID=%d)"), *GetName(), SkillID);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
}
