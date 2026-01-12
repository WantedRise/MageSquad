// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Enemy/Notify/MSAN_SendGameplayEvent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

UMSAN_SendGameplayEvent::UMSAN_SendGameplayEvent()
{
}

void UMSAN_SendGameplayEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (!Owner) return;

	UAbilitySystemComponent* ASC = 
		UAbilitySystemBlueprintLibrary
	::GetAbilitySystemComponent(Owner);
	if (!ASC) return;

	FGameplayEventData Payload;
	Payload.Instigator = Owner;
	ASC->HandleGameplayEvent(EventTag, &Payload);
}
