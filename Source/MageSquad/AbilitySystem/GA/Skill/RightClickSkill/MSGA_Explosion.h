// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GA/Skill/MSGA_SkillBase.h"
#include "MSGA_Explosion.generated.h"

/**
*  ?묒꽦?? 諛뺤꽭李?
 * ?묒꽦?? 25/12/16
 * 
 * ?고겢由?쑝濡?諛쒕룞?섎뒗 ?ㅽ궗 以??섎굹
 * 留덉슦???꾩튂????컻???쇱쑝??
 */
UCLASS()
class MAGESQUAD_API UMSGA_Explosion : public UMSGA_SkillBase
{
	GENERATED_BODY()
	
public:
	UMSGA_Explosion();
	
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
	// Explosion ?ㅽ궗 ID??21
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentSkillID = 21;
	
	// ?쇳빐??
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<float> BaseDamageSequence = { 0.1f, 0.1f, 0.1f, 0.7f };
	
	
	// ?쇳빐 二쇨린
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DamageInterval = 0.25f;

	// 荑⑦???
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CoolTime = 3.f;

	// ?ㅽ궗 踰붿쐞
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Range = 1.f;


};
