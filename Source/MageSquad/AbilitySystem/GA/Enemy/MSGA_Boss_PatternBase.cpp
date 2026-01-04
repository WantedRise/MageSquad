// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GA/Enemy/MSGA_Boss_PatternBase.h"

#include "AbilitySystemComponent.h"
#include "MSGameplayTags.h"
#include "Enemy/MSBossEnemy.h"

UMSGA_Boss_PatternBase::UMSGA_Boss_PatternBase()
{
	// 활성화 시 Owner에게 부여되는 Tag
	ActivationOwnedTags.AddTag(MSGameplayTags::Enemy_State_PreventRotation);
}

void UMSGA_Boss_PatternBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UMSGA_Boss_PatternBase::CancelAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UMSGA_Boss_PatternBase::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
	UAbilitySystemComponent* ASC = Owner->GetAbilitySystemComponent();	
	// GameplayEffectSpec 생성
	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddSourceObject(Owner);
	
	FGameplayEffectSpecHandle SpecHandle =
		ASC->MakeOutgoingSpec(Owner->GetCooldownEffectClass(), 1.f, Context);

	if (!SpecHandle.IsValid())
	{
		return;
	}
	
	// GameplayEffect 적용
	ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
}
