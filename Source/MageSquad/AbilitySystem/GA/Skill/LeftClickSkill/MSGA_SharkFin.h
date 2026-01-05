// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GA/Skill/MSGA_SkillBase.h"
#include "MSGA_SharkFin.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UMSGA_SharkFin : public UMSGA_SkillBase
{
	GENERATED_BODY()

public:
	UMSGA_SharkFin();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
	virtual void ApplyCooldown(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const override;

protected:
	// SharkFin ?ㅽ궗 ID??11
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentSkillID = 11;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CoolTime = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Range = 1.f;


};
