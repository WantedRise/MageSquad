// Fill out your copyright notice in the Description page of Project Settings.


#include "MSFunctionLibrary.h"
#include "AbilitySystem/ASC/MSPlayerAbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

#include "MSGameplayTags.h"

UMSPlayerAbilitySystemComponent* UMSFunctionLibrary::NativeGetPlayerAbilitySystemComponentFromActor(AActor* InActor)
{
	if (!InActor)
	{
		return nullptr;
	}

	if (UMSPlayerAbilitySystemComponent* ASC = Cast<UMSPlayerAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InActor)))
	{
		return ASC;
	}
	return nullptr;
}

bool UMSFunctionLibrary::NativeDoesActorHaveTag(AActor* InActor, const FGameplayTag TagToCheck)
{
	if (UMSPlayerAbilitySystemComponent* ASC = NativeGetPlayerAbilitySystemComponentFromActor(InActor))
	{
		return ASC->HasMatchingGameplayTag(TagToCheck);
	}
	return false;
}
