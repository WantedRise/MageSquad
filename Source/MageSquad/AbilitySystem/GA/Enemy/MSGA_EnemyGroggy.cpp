// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GA/Enemy/MSGA_EnemyGroggy.h"

#include "AbilitySystemComponent.h"
#include "MSGameplayTags.h"
#include "AbilitySystem/Tasks/MSAT_PlayMontageAndWaitForEvent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "Enemy/MSBossEnemy.h"
#include "Enemy/AIController/MSBossAIController.h"


UMSGA_EnemyGroggy::UMSGA_EnemyGroggy()
{
	// 어빌리티 식별 Tag
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(MSGameplayTags::Enemy_Ability_Groggy);
	SetAssetTags(TagContainer);

	// 활성화 시 Owner에게 부여되는 Tag
	ActivationOwnedTags.AddTag(MSGameplayTags::Enemy_State_Groggy);
	
	BlockAbilitiesWithTag.AddTag(MSGameplayTags::Enemy_Ability_Dead)	;
}

void UMSGA_EnemyGroggy::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	GroggyCountAcc = 0;
	
	if (UAnimMontage* GroggyMontage = Owner->GetGroggyMontage())
	{
		FGameplayTagContainer Tags;
		Tags.AddTag(FGameplayTag::RequestGameplayTag("Enemy.Event.Groggy"));

		UMSAT_PlayMontageAndWaitForEvent* AttackTask = 
			UMSAT_PlayMontageAndWaitForEvent::CreateTask(this, GroggyMontage, Tags);

		AttackTask->OnCompleted.AddDynamic(this, &UMSGA_EnemyGroggy::OnCompleteCallback);
		AttackTask->OnInterrupted.AddDynamic(this, &UMSGA_EnemyGroggy::OnInterruptedCallback);
		AttackTask->OnEventReceived.AddDynamic(this, &UMSGA_EnemyGroggy::OnEventReceivedCallback);
		AttackTask->ReadyForActivation();
	}
	
	Owner->SetActorEnableCollision(false);
}
	
void UMSGA_EnemyGroggy::CancelAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UMSGA_EnemyGroggy::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
	if (AMSBossAIController* EnemyAIController = Cast<AMSBossAIController>(Owner->GetController()))
	{
		EnemyAIController->GetBlackboardComponent()->SetValueAsBool(EnemyAIController->GetIsGroggyKey(), false);
		Owner->GetAbilitySystemComponent()->AddLooseGameplayTag(MSGameplayTags::Enemy_State_Phase2);
		Owner->SetActorEnableCollision(true);
	}
}

void UMSGA_EnemyGroggy::OnCompleteCallback()
{
	bool bReplicatedEndAbility = true;
	bool bWasCancelled = false;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicatedEndAbility, bWasCancelled);
}

void UMSGA_EnemyGroggy::OnInterruptedCallback()
{
	bool bReplicatedEndAbility = true;
	bool bWasCancelled = true;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicatedEndAbility, bWasCancelled);
}

void UMSGA_EnemyGroggy::OnEventReceivedCallback(FGameplayTag EventTag, FGameplayEventData EventData)
{
	if (GroggyCountAcc >= 5)
	{
		UAnimInstance* AnimInstance = Owner->GetMesh()->GetAnimInstance();
		AnimInstance->Montage_JumpToSection(FName(TEXT("End")), AnimInstance->GetCurrentActiveMontage());
	}
	
	else
	{
		++GroggyCountAcc;
	}	
}
