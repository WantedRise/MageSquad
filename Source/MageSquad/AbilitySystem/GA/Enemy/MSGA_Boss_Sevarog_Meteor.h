// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GA/Enemy/MSGA_EnemyBossBaseAbility.h"
#include "MSGA_Boss_Sevarog_Meteor.generated.h"

/**
 * 작성자 : 임희섭
 * 작성일 : 2026/01/02
 * Boss Sevarog의 Pattern1 어빌리티
 * 플레이어를 추격하며 바닥이 터진다. 
 */
UCLASS()
class MAGESQUAD_API UMSGA_Boss_Sevarog_Meteor : public UMSGA_EnemyBossBaseAbility
{
	GENERATED_BODY()
	
public:
	UMSGA_Boss_Sevarog_Meteor();
	
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
