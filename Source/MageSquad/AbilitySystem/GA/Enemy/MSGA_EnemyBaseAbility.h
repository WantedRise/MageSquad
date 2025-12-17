// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "MSGA_EnemyBaseAbility.generated.h"

/**
 * 작성자 : 임희섭
 * 작성일 : 2025/12/17
 * Enemy들이 가질 Ability의 base 클래스
 */
UCLASS()
class MAGESQUAD_API UMSGA_EnemyBaseAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UMSGA_EnemyBaseAbility();
	
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
	
protected:
	UPROPERTY()
	TObjectPtr<class AMSBaseEnemy> Owner;
	
	
	
};
