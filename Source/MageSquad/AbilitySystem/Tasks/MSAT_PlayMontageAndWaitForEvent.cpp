// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Tasks/MSAT_PlayMontageAndWaitForEvent.h"

#include "AbilitySystemComponent.h"

UMSAT_PlayMontageAndWaitForEvent::UMSAT_PlayMontageAndWaitForEvent()
{
}

UMSAT_PlayMontageAndWaitForEvent* UMSAT_PlayMontageAndWaitForEvent::CreateTask(UGameplayAbility* OwningAbility,
	UAnimMontage* Montage, FGameplayTagContainer EventTags, float Rate, const FName& StartSectionName)
{
	UMSAT_PlayMontageAndWaitForEvent* Task = 
	NewAbilityTask<UMSAT_PlayMontageAndWaitForEvent>(OwningAbility);

	Task->Montage = Montage;
	Task->EventTags = EventTags;
	Task->Rate = Rate;
	Task->StartSectionName = StartSectionName;

	return Task;
}

void UMSAT_PlayMontageAndWaitForEvent::Activate()
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	UAnimInstance* AnimInstance = Ability ? Ability->GetCurrentActorInfo()->GetAnimInstance() : nullptr;

	if (!ASC || !AnimInstance || !Montage)
	{
		EndTask();
		return;
	}

	// 이벤트 등록
	EventHandle = ASC->AddGameplayEventTagContainerDelegate(
		EventTags,
		FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(
			this, &ThisClass::OnGameplayEvent));

	// 몽타주 재생
	if (ASC->PlayMontage(Ability, Ability->GetCurrentActivationInfo(), Montage, Rate, StartSectionName) > 0.0f)
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &ThisClass::OnMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);
	}
	else
	{
		EndTask();
	}
}

void UMSAT_PlayMontageAndWaitForEvent::OnDestroy(bool bInOwnerFinished)
{
	if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
	{
		ASC->RemoveGameplayEventTagContainerDelegate(EventTags, EventHandle);
        
		if (bInOwnerFinished)
		{
			ASC->CurrentMontageStop();
		}
	}

	Super::OnDestroy(bInOwnerFinished);
}

void UMSAT_PlayMontageAndWaitForEvent::OnMontageEnded(UAnimMontage* EndedMontage, bool bInterrupted)
{
	if (EndedMontage != Montage)
	{
		return;
	}
	
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		if (bInterrupted)
		{
			OnInterrupted.Broadcast();
		}
		else
		{
			OnCompleted.Broadcast();
		}
	}

	EndTask();
}

void UMSAT_PlayMontageAndWaitForEvent::OnGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload) const
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnEventReceived.Broadcast(EventTag, Payload ? *Payload : FGameplayEventData());
	}
}
