// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GA/Enemy/MSGA_EnemyMove.h"

#include "MSGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Enemy/MSBaseEnemy.h"

UMSGA_EnemyMove::UMSGA_EnemyMove()
{
	// 어빌리티 식별 Tag
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(MSGameplayTags::Enemy_Ability_Move);
	SetAssetTags(TagContainer);

	// 활성화 시 Owner에게 부여되는 Tag
	ActivationOwnedTags.AddTag(MSGameplayTags::Enemy_State_Move);
	
	BlockAbilitiesWithTag.AddTag(MSGameplayTags::Enemy_Ability_Move)	;
}

void UMSGA_EnemyMove::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (UAnimMontage* MoveMontage = Owner->GetMoveMontage())
	{
		UAbilityTask_PlayMontageAndWait* EnemyMoveTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("Move"), MoveMontage);
		EnemyMoveTask->OnCompleted.AddDynamic(this, &UMSGA_EnemyMove::OnCompleteCallback); // 몽타주가 끝나면 호출될 함수
		EnemyMoveTask->OnInterrupted.AddDynamic(this, &UMSGA_EnemyMove::OnInterruptedCallback); // 몽타주가 중단되면 호출될 함수
		EnemyMoveTask->ReadyForActivation();
	}
}

void UMSGA_EnemyMove::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UMSGA_EnemyMove::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UMSGA_EnemyMove::OnCompleteCallback()
{
	bool ReplicatedEndAbility = true;
	bool WasCancelled = false;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, ReplicatedEndAbility, WasCancelled);	
}

void UMSGA_EnemyMove::OnInterruptedCallback()
{
	bool ReplicatedEndAbility = true;
	bool WasCancelled = true;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, ReplicatedEndAbility, WasCancelled);	
}
