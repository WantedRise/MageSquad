// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GA/Skill/MSGA_SkillBase.h"
#include "MSGA_ChainBolt.generated.h"

class UProjectileStaticData;

UCLASS()
class MAGESQUAD_API UMSGA_ChainBolt : public UMSGA_SkillBase
{
	GENERATED_BODY()

public:
	UMSGA_ChainBolt();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentSkillID = 6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Range = 800.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ChainBolt")
	TSubclassOf<UProjectileStaticData> LightningProjectileDataClass;
};
