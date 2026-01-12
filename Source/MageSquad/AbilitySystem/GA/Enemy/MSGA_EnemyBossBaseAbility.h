// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "MSGA_EnemyBossBaseAbility.generated.h"

/**
 * 작성자 : 임희섭
 * 작성일 : 2025/12/22
 * 보스 전용 어빌리티의 Base 클래스
 */
UCLASS()
class MAGESQUAD_API UMSGA_EnemyBossBaseAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UMSGA_EnemyBossBaseAbility();
	
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
	TObjectPtr<class AMSBossEnemy> Owner;
	
};
