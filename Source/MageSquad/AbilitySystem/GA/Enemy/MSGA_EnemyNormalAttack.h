// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GA/Enemy/MSGA_EnemyBaseAbility.h"
#include "MSGA_EnemyNormalAttack.generated.h"

/**
 * 작성자 : 임희섭
 * 작성일 : 2025/12/17
 * Enemy들이 가질 기본 공격을 할 때 발동될 어빌리티
 */
UCLASS()
class MAGESQUAD_API UMSGA_EnemyNormalAttack : public UMSGA_EnemyBaseAbility
{
	GENERATED_BODY()
	
public:
	UMSGA_EnemyNormalAttack();

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
