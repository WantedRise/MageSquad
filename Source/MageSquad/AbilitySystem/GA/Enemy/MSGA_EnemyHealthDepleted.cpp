// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GA/Enemy/MSGA_EnemyHealthDepleted.h"

#include "AbilitySystemComponent.h"
#include "MSGameplayTags.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Enemy/MSBaseEnemy.h"
#include "Enemy/MSBossEnemy.h"
#include "Enemy/AIController/MSBaseAIController.h"
#include "Enemy/AIController/MSBossAIController.h"

UMSGA_EnemyHealthDepleted::UMSGA_EnemyHealthDepleted()
{
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = MSGameplayTags::Enemy_Event_HealthDepleted;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);

	BlockAbilitiesWithTag.AddTag(MSGameplayTags::Enemy_Ability_Groggy);
	
	ActivationBlockedTags.AddTag(MSGameplayTags::Enemy_Event_HealthDepleted);
}

void UMSGA_EnemyHealthDepleted::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
    
	const bool bIsBossTag = ASC->HasMatchingGameplayTag(MSGameplayTags::Enemy_Tier_Boss);
	const bool bIsBossClass = Owner->IsA(AMSBossEnemy::StaticClass());
	const bool bIsBoss = bIsBossTag || bIsBossClass;
	
	const bool bIsPhase2 = ASC->HasMatchingGameplayTag(MSGameplayTags::Enemy_State_Phase2);

	if (!bIsBoss || bIsPhase2)
	{
		if (AMSBaseAIController* AIController = Cast<AMSBaseAIController>(Owner->GetController()))
		{
			AIController->GetBlackboardComponent()->SetValueAsBool(AIController->GetIsDeadKey(), true);
			
			if(Owner->IsA(AMSBossEnemy::StaticClass()))
			{
				UE_LOG(LogTemp, Error, TEXT("Boss Is Dead: Phase2 %s"), bIsPhase2 ? TEXT("true") : TEXT("false"));
			}
		}
	}
	else
	{
	 	if (AMSBossAIController* AIController = Cast<AMSBossAIController>(Owner->GetController()))
	 	{
		    if (!ASC->HasMatchingGameplayTag(MSGameplayTags::Enemy_State_Groggy))
		    {
			    AIController->GetBlackboardComponent()->SetValueAsBool(AIController->GetIsGroggyKey(), true);
			    UE_LOG(LogTemp, Error, TEXT("Boss Is Groggy"));
		    }
	 	}
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