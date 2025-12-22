// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GA/Enemy/MSGA_EnemySpawn.h"

#include "MSGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Enemy/MSBaseEnemy.h"
#include "Enemy/MSBossEnemy.h"
#include "Enemy/AIController/MSBossAIController.h"

UMSGA_EnemySpawn::UMSGA_EnemySpawn()
{
	// 어빌리티 식별 Tag
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(MSGameplayTags::Enemy_Ability_Spawn);
	SetAssetTags(TagContainer);

	// 활성화 시 Owner에게 부여되는 Tag
	ActivationOwnedTags.AddTag(MSGameplayTags::Enemy_State_Spawn);
}

void UMSGA_EnemySpawn::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (UAnimMontage* SpawnMontage = Owner->GetSpawnMontage())
	{
		UAbilityTask_PlayMontageAndWait* EnemySpawnTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("Spawn"), SpawnMontage);
		EnemySpawnTask->OnCompleted.AddDynamic(this, &UMSGA_EnemySpawn::OnCompleteCallback); // 몽타주가 끝나면 호출될 함수
		EnemySpawnTask->OnInterrupted.AddDynamic(this, &UMSGA_EnemySpawn::OnInterruptedCallback); // 몽타주가 중단되면 호출될 함수
		EnemySpawnTask->ReadyForActivation();
	}
}

void UMSGA_EnemySpawn::CancelAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UMSGA_EnemySpawn::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
	if (AMSBossAIController* BossAIController = Cast<AMSBossAIController>(Owner->GetController()))
	{
		BossAIController->GetBlackboardComponent()->SetValueAsBool(TEXT("IsSpawnd"), false);
	}
}

void UMSGA_EnemySpawn::OnCompleteCallback()
{
	bool bReplicatedEndAbility = true;
	bool bWasCancelled = false;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicatedEndAbility, bWasCancelled);
}

void UMSGA_EnemySpawn::OnInterruptedCallback()
{
	bool bReplicatedEndAbility = true;
	bool bWasCancelled = true;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicatedEndAbility, bWasCancelled);
}
