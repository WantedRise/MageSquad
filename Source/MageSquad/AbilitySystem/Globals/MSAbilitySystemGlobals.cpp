// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Globals/MSAbilitySystemGlobals.h"
#include "Types/MageSquadTypes.h"

FGameplayEffectContext* UMSAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FMSGameplayEffectContext();
}
