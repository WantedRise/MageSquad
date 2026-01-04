// Fill out your copyright notice in the Description page of Project Settings.


#include "MSGA_Boss_Sevarog_Laser.h"

#include "AbilitySystemComponent.h"
#include "MSGameplayTags.h"
#include "AbilitySystem/Tasks/MSAT_PlayMontageAndWaitForEvent.h"
#include "Enemy/MSBossEnemy.h"

UMSGA_Boss_Sevarog_Laser::UMSGA_Boss_Sevarog_Laser()
{
	// 어빌리티 식별 Tag
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(MSGameplayTags::Enemy_Ability_Pattern2);
	SetAssetTags(TagContainer);

	// 활성화 시 Owner에게 부여되는 Tag
	ActivationOwnedTags.AddTag(MSGameplayTags::Enemy_State_Pattern2);
}

void UMSGA_Boss_Sevarog_Laser::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (UAnimMontage* Pattern2Montage = Owner->GetPattern2Montage())
	{
		FGameplayTagContainer Tags;
		Tags.AddTag(FGameplayTag::RequestGameplayTag("Enemy.Event.NormalAttack"));
		
		// Todo : 추후에 페이즈 전환 관련 델리게이트로 빼서 관리할 예정
		FName StartSectionName = NAME_None;
		// if (Owner->GetAbilitySystemComponent()->HasMatchingGameplayTag(MSGameplayTags::Enemy_State_Phase2))
		// {
		// 	StartSectionName = TEXT("Phase2");
		// }

		UMSAT_PlayMontageAndWaitForEvent* Pattern2Task = 
			UMSAT_PlayMontageAndWaitForEvent::CreateTask(this, Pattern2Montage, Tags, 1.f, StartSectionName);

		Pattern2Task->OnCompleted.AddDynamic(this, &UMSGA_Boss_Sevarog_Laser::OnCompleteCallback);
		Pattern2Task->OnInterrupted.AddDynamic(this, &UMSGA_Boss_Sevarog_Laser::OnInterruptedCallback);
		//Pattern2Task->OnEventReceived.AddDynamic(this, &UMSGA_Boss_Sevarog_Laser::OnEventReceivedCallback);
		Pattern2Task->ReadyForActivation();
	}
}

void UMSGA_Boss_Sevarog_Laser::CancelAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UMSGA_Boss_Sevarog_Laser::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UMSGA_Boss_Sevarog_Laser::OnCompleteCallback()
{
	bool bReplicatedEndAbility = true;
	bool bWasCancelled = false;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicatedEndAbility, bWasCancelled);
}

void UMSGA_Boss_Sevarog_Laser::OnInterruptedCallback()
{
	bool bReplicatedEndAbility = true;
	bool bWasCancelled = true;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicatedEndAbility, bWasCancelled);
}
