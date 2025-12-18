// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GA/Enemy/MSGA_EnemyDead.h"

#include "MSGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Enemy/MSBaseEnemy.h"

UMSGA_EnemyDead::UMSGA_EnemyDead()
{
	// 어빌리티 식별 Tag
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(MSGameplayTags::Enemy_Ability_Dead);
	SetAssetTags(TagContainer);

	// 활성화 시 Owner에게 부여되는 Tag
	ActivationOwnedTags.AddTag(MSGameplayTags::Enemy_State_Dead);
}

void UMSGA_EnemyDead::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (UAnimMontage* AttackMontage = Owner->GetAttackMontage())
	{
		UAbilityTask_PlayMontageAndWait* EnemyAttackTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("NormalAttack"), AttackMontage);
		EnemyAttackTask->OnCompleted.AddDynamic(this, &UMSGA_EnemyDead::OnCompleteCallback); // 몽타주가 끝나면 호출될 함수
		EnemyAttackTask->OnInterrupted.AddDynamic(this, &UMSGA_EnemyDead::OnInterruptedCallback); // 몽타주가 중단되면 호출될 함수
		EnemyAttackTask->ReadyForActivation();
	}
}

void UMSGA_EnemyDead::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UMSGA_EnemyDead::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UMSGA_EnemyDead::OnCompleteCallback()
{
	bool bReplicatedEndAbility = true;
	bool bWasCancelled = true;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicatedEndAbility, bWasCancelled);
}

void UMSGA_EnemyDead::OnInterruptedCallback()
{
	bool bReplicatedEndAbility = true;
	bool bWasCancelled = true;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicatedEndAbility, bWasCancelled);
}
