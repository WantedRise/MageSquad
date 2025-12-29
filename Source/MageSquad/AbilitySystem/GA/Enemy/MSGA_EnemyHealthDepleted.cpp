// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GA/Enemy/MSGA_EnemyHealthDepleted.h"

#include "AbilitySystemComponent.h"
#include "MSGameplayTags.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Enemy/MSBaseEnemy.h"
#include "Enemy/AIController/MSBaseAIController.h"
#include "Enemy/AIController/MSBossAIController.h"

UMSGA_EnemyHealthDepleted::UMSGA_EnemyHealthDepleted()
{
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = MSGameplayTags::Enemy_Event_HealthDepleted;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);

	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UMSGA_EnemyHealthDepleted::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
    
	const bool bIsBoss = ASC->HasMatchingGameplayTag(MSGameplayTags::Enemy_Tier_Boss);
	const bool bIsPhase2 = ASC->HasMatchingGameplayTag(MSGameplayTags::Enemy_State_Phase2);

	if (!bIsBoss || bIsPhase2)
	{
		if (AMSBaseAIController* AIController = Cast<AMSBaseAIController>(Owner->GetController()))
		{
			AIController->GetBlackboardComponent()->SetValueAsBool(AIController->GetIsDeadKey(), true);
		}
		//HandleDeath();
	}
	 else
	 {
	 	if (AMSBossAIController* AIController = Cast<AMSBossAIController>(Owner->GetController()))
	 	{
	 		AIController->GetBlackboardComponent()->SetValueAsBool(AIController->GetIsGroggyKey(), true);
	 	}
	 	//HandleBossPhaseTransition(ASC);
	 }

	bool bReplicatedEndAbility = true;
	bool bWasCancelled = false;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicatedEndAbility, bWasCancelled);
}

void UMSGA_EnemyHealthDepleted::CancelAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UMSGA_EnemyHealthDepleted::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}