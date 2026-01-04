// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GA/Enemy/MSGA_EnemyBossBaseAbility.h"
#include "MSGA_Boss_PatternBase.generated.h"

/**
 * 작성자 : 임희섭
 * 작성일 : 2026/01/04
 * 보스의 패턴들의 공통 기능이 들어가는 어빌리티
 */
UCLASS()
class MAGESQUAD_API UMSGA_Boss_PatternBase : public UMSGA_EnemyBossBaseAbility
{
	GENERATED_BODY()
	
public:
	UMSGA_Boss_PatternBase();
	
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
	
};
