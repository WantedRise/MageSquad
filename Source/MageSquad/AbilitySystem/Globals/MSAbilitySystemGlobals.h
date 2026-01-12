// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "MSAbilitySystemGlobals.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UMSAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()
	
public:
	/** 커스텀 GameplayEffectContext 할당 */
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
	
};
