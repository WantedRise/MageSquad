// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GA/Enemy/MSGA_EnemyBaseAbility.h"
#include "MSGA_EnemyHealthDepleted.generated.h"

/**
 * 작성자 : 임희섭
 * 작성일 : 2025/12/25
 * Enemy의 Health가 소진됐을 때 발동하는 어빌리티
 * Normal Enemy와 Boss Enemy의 Event를 나누기 위함
 */
UCLASS()
class MAGESQUAD_API UMSGA_EnemyHealthDepleted : public UMSGA_EnemyBaseAbility
{
	GENERATED_BODY()
	
public:
	UMSGA_EnemyHealthDepleted();
	
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
