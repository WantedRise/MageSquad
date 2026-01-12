// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GA/Enemy/MSGA_EnemyBaseAbility.h"
#include "MSGA_EnemyIdle.generated.h"

/**
 * 작성자 : 임희섭
 * 작성일 : 2025/12/20
 * Enemy의 기본 상태를 나타낼 Abilty
 */
UCLASS()
class MAGESQUAD_API UMSGA_EnemyIdle : public UMSGA_EnemyBaseAbility
{
	GENERATED_BODY()
	
public:
	UMSGA_EnemyIdle();
	
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
