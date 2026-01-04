// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MSGA_Boss_PatternBase.h"
#include "AbilitySystem/GA/Enemy/MSGA_EnemyBossBaseAbility.h"
#include "MSGA_Boss_Sevarog_Grip.generated.h"

/**
 * 작성자 : 임희섭
 * 작성일 : 2026/01/03
 * Boss Sevarog의 Pattern3 어빌리티
 * 전방 부채꼴의 범위에 피해를 준다. 
 */
UCLASS()
class MAGESQUAD_API UMSGA_Boss_Sevarog_Grip : public UMSGA_Boss_PatternBase
{
	GENERATED_BODY()
	
public:
	UMSGA_Boss_Sevarog_Grip();
	
public:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void CancelAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateCancelAbility) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

private:
	UFUNCTION()
	void OnCompleteCallback();
	UFUNCTION()
	void OnInterruptedCallback();
};
