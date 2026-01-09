// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GA/Enemy/MSGA_EnemyMove.h"

#include "AIController.h"
#include "MSGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/Tasks/MSAT_EnemyMove.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Enemy/MSBaseEnemy.h"
#include "Enemy/AIController/MSBaseAIController.h"

UMSGA_EnemyMove::UMSGA_EnemyMove()
{
	// 어빌리티 식별 Tag
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(MSGameplayTags::Enemy_Ability_Move);
	SetAssetTags(TagContainer);

	// 활성화 시 Owner에게 부여되는 Tag
	ActivationOwnedTags.AddTag(MSGameplayTags::Enemy_State_Move);
	
	ActivationBlockedTags.AddTag(MSGameplayTags::Enemy_Ability_NormalAttack);
}

void UMSGA_EnemyMove::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	// Move Task 생성 (서버에서만 Tick 실행됨)
	UMSAT_EnemyMove* MoveTask = UMSAT_EnemyMove::CreateTask(
		this,
		AcceptanceRadius,
		GetTargetUpdateInterval());
    
	MoveTask->OnTargetReached.AddDynamic(this, &UMSGA_EnemyMove::OnCompleteCallback);
	MoveTask->OnTargetLost.AddDynamic(this, &UMSGA_EnemyMove::OnInterruptedCallback);
	MoveTask->ReadyForActivation();
	
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
	
	if (AMSBaseAIController* AIC = Cast<AMSBaseAIController>(ActorInfo->OwnerActor->GetInstigatorController()))
	{
		AIC->StopMovement();
	}
}

void UMSGA_EnemyMove::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
	if (AMSBaseAIController* AIC = Cast<AMSBaseAIController>(ActorInfo->OwnerActor->GetInstigatorController()))
	{
		AIC->StopMovement();
	}
}

float UMSGA_EnemyMove::GetTargetUpdateInterval() const
{
	if (!Owner)
	{
		return 0.25f;
	}
	
	return 0.3f;    // 일반: 느린 반응 (대량 스폰 최적화)
	
	// switch (Owner->GetEnemyTier())
	// {
	// case EEnemyTier::Boss:
	// 	return 0.1f;    // 보스: 빠른 반응
	// case EEnemyTier::Elite:
	// 	return 0.2f;    // 엘리트: 중간
	// case EEnemyTier::Normal:
	// default:
	// }
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
