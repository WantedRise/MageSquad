// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GA/Enemy/MSGA_EnemyIdle.h"

#include "MSGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Enemy/MSBaseEnemy.h"

UMSGA_EnemyIdle::UMSGA_EnemyIdle()
{
	// 어빌리티 식별 Tag
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(MSGameplayTags::Enemy_Ability_Idle);
	SetAssetTags(TagContainer);

	// 활성화 시 Owner에게 부여되는 Tag
	ActivationOwnedTags.AddTag(MSGameplayTags::Enemy_State_Idle);
	
	BlockAbilitiesWithTag.AddTag(MSGameplayTags::Enemy_Ability_Idle)	;
}

void UMSGA_EnemyIdle::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (UAnimMontage* IdleMontage = Owner->GetIdleMontage())
	{
		UAbilityTask_PlayMontageAndWait* EnemyDeadTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("Idle"), IdleMontage);
		EnemyDeadTask->OnCompleted.AddDynamic(this, &UMSGA_EnemyIdle::OnCompleteCallback); // 몽타주가 끝나면 호출될 함수
		EnemyDeadTask->OnInterrupted.AddDynamic(this, &UMSGA_EnemyIdle::OnInterruptedCallback); // 몽타주가 중단되면 호출될 함수
		EnemyDeadTask->ReadyForActivation();
		UE_LOG(LogTemp, Warning, TEXT("[%s] Enemy Dead Ability Being"), *GetAvatarActorFromActorInfo()->GetName())
	}
}

void UMSGA_EnemyIdle::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UMSGA_EnemyIdle::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UMSGA_EnemyIdle::OnCompleteCallback()
{
	bool bReplicatedEndAbility = true;
	bool bWasCancelled = false;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicatedEndAbility, bWasCancelled);
}

void UMSGA_EnemyIdle::OnInterruptedCallback()
{
	bool bReplicatedEndAbility = true;
	bool bWasCancelled = true;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicatedEndAbility, bWasCancelled);
}
