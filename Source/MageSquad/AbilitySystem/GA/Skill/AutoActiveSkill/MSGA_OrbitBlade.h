// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GA/Skill/MSGA_SkillBase.h"
#include "MSGA_OrbitBlade.generated.h"

UCLASS()
class MAGESQUAD_API UMSGA_OrbitBlade : public UMSGA_SkillBase
{
	GENERATED_BODY()

public:
	UMSGA_OrbitBlade();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentSkillID = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Range = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OrbitSpeedDeg = 180.f;
};
