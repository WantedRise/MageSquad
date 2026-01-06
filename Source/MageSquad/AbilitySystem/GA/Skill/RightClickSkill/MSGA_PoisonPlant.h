// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GA/Skill/MSGA_SkillBase.h"
#include "MSGA_PoisonPlant.generated.h"

UCLASS()
class MAGESQUAD_API UMSGA_PoisonPlant : public UMSGA_SkillBase
{
	GENERATED_BODY()

public:
	UMSGA_PoisonPlant();

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
	// PoisonPlant ?ㅽ궗 ID??22
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentSkillID = 22;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<float> BaseDamageSequence = { 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DamageInterval = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CoolTime = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Range = 1.f;


};
