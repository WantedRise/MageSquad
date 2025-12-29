// Fill out your copyright notice in the Description page of Project Settings.


#include "MSGA_SkillBase.h"
#include "Player/MSPlayerState.h"

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
	// PlayerState 가져오기
	APawn* Pawn = Cast<APawn>(ActorInfo->AvatarActor.Get());
	if (!Pawn)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AMSPlayerState* PS = Pawn->GetPlayerState<AMSPlayerState>();
	if (!PS)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	const FMSSkillList* Found = PS->GetOwnedSkillByID(SkillID);
	if (!Found)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	SkillListRow = *Found;
}
