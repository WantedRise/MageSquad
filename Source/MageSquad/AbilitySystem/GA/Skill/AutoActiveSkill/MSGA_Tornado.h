// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GA/Skill/MSGA_SkillBase.h"
#include "MSGA_Tornado.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UMSGA_Tornado : public UMSGA_SkillBase
{
	GENERATED_BODY()
	
public:
	UMSGA_Tornado();
	
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
	
protected:
	// Tornado ?ㅽ궗 ID??3
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentSkillID = 3;
	
	// ?쇳빐??
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SkillDamage = 10.f;
	
	// 踰붿쐞
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SkillRadius = 200.f;
	
	// 吏?띿떆媛?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SkillDuration = 8.f;
};
