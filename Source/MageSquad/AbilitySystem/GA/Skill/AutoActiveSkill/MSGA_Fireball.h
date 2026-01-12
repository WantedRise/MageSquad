// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GA/Skill/MSGA_SkillBase.h"
#include "MSGA_Fireball.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UMSGA_Fireball : public UMSGA_SkillBase
{
	GENERATED_BODY()

public:
	UMSGA_Fireball();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentSkillID = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SkillDamage = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SkillRadius = 200.f;
};
