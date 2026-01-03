// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GA/Enemy/MSGA_Boss_Sevarog_Grip.h"

#include "MSGameplayTags.h"
#include "AbilitySystem/Tasks/MSAT_PlayMontageAndWaitForEvent.h"
#include "Enemy/MSBossEnemy.h"

UMSGA_Boss_Sevarog_Grip::UMSGA_Boss_Sevarog_Grip()
{
	// 어빌리티 식별 Tag
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(MSGameplayTags::Enemy_Ability_Pattern3);
	SetAssetTags(TagContainer);

	// 활성화 시 Owner에게 부여되는 Tag
	ActivationOwnedTags.AddTag(MSGameplayTags::Enemy_State_Pattern3);
}

void UMSGA_Boss_Sevarog_Grip::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (UAnimMontage* Pattern3Montage = Owner->GetPattern3Montage())
	{
		FGameplayTagContainer Tags;
		Tags.AddTag(FGameplayTag::RequestGameplayTag("Enemy.Event.NormalAttack"));
		
		// Todo : 추후에 페이즈 전환 관련 델리게이트로 빼서 관리할 예정
		FName StartSectionName = NAME_None;
		// if (Owner->GetAbilitySystemComponent()->HasMatchingGameplayTag(MSGameplayTags::Enemy_State_Phase2))
		// {
		// 	StartSectionName = TEXT("Phase2");
		// }

		UMSAT_PlayMontageAndWaitForEvent* Pattern3Task = 
			UMSAT_PlayMontageAndWaitForEvent::CreateTask(this, Pattern3Montage, Tags, 1.f, StartSectionName);

		Pattern3Task->OnCompleted.AddDynamic(this, &UMSGA_Boss_Sevarog_Grip::OnCompleteCallback);
		Pattern3Task->OnInterrupted.AddDynamic(this, &UMSGA_Boss_Sevarog_Grip::OnInterruptedCallback);
		//Pattern2Task->OnEventReceived.AddDynamic(this, &UMSGA_Boss_Sevarog_Grip::OnEventReceivedCallback);
		Pattern3Task->ReadyForActivation();
	}
}

void UMSGA_Boss_Sevarog_Grip::CancelAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UMSGA_Boss_Sevarog_Grip::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UMSGA_Boss_Sevarog_Grip::OnCompleteCallback()
{
	bool bReplicatedEndAbility = true;
	bool bWasCancelled = false;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicatedEndAbility, bWasCancelled);
}

void UMSGA_Boss_Sevarog_Grip::OnInterruptedCallback()
{
	bool bReplicatedEndAbility = true;
	bool bWasCancelled = true;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicatedEndAbility, bWasCancelled);
}
