// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MSGA_EnemyBossBaseAbility.h"
#include "AbilitySystem/GA/Enemy/MSGA_EnemyBaseAbility.h"
#include "MSGA_EnemySpawn.generated.h"

/**
 * 작성자 : 임희섭
 * 작성일 : 2025/12/22
 * 보스가 등장하는 어빌리티.
 */
UCLASS()
class MAGESQUAD_API UMSGA_EnemySpawn : public UMSGA_EnemyBossBaseAbility
{
	GENERATED_BODY()
	
public:
	UMSGA_EnemySpawn();

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
