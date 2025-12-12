// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GA/AutoActiveSkill/MSGA_AutoActiveSkillBase.h"
#include "MSGA_IceSpear.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UMSGA_IceSpear : public UMSGA_AutoActiveSkillBase
{
	GENERATED_BODY()
	
public:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
	
protected:
	// Ice Spear 스킬 ID는 1
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SkillID = 1;
	
	// 피해량
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SkillDamage = 20.f;

	// 쿨타임
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CoolTime = 3.f;

	// 투사체 개수
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ProjectileNumber = 1;

	// 관통 횟수
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Penetration = 0;
};