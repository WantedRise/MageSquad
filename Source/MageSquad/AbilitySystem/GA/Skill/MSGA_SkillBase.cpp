// Fill out your copyright notice in the Description page of Project Settings.


#include "MSGA_SkillBase.h"
#include "Player/MSPlayerState.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AttributeSets/MSPlayerAttributeSet.h"

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

void UMSGA_SkillBase::ApplyPlayerCritToRuntimeData(
	const FGameplayAbilityActorInfo* ActorInfo,
	FProjectileRuntimeData& RuntimeData) const
{
	if (!ActorInfo)
	{
		return;
	}

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC)
	{
		return;
	}

	const UMSPlayerAttributeSet* AttributeSet =
		Cast<UMSPlayerAttributeSet>(ASC->GetAttributeSet(UMSPlayerAttributeSet::StaticClass()));
	if (!AttributeSet)
	{
		return;
	}

	RuntimeData.CriticalChance = AttributeSet->GetCritChance();
	RuntimeData.CriticalDamage = AttributeSet->GetCritDamage();

	const float DamageModPercent = AttributeSet->GetDamageMod();
	const float DamageScale = 1.0f + (DamageModPercent / 100.0f);
	RuntimeData.Damage *= DamageScale;
	for (float& DamageValue : RuntimeData.DamageSequence)
	{
		DamageValue *= DamageScale;
	}
}
