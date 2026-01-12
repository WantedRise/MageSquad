// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MSGA_EnemyBossBaseAbility.h"
#include "MSGA_EnemyGroggy.generated.h"

/**
 * 작성자 : 임희섭
 * 작성일 : 2025/12/25
 * 보스의 페이즈 전환 시 Groggy 상태를 나타낼 때 발동할 Ability
 */
UCLASS()
class MAGESQUAD_API UMSGA_EnemyGroggy : public UMSGA_EnemyBossBaseAbility
{
	GENERATED_BODY()
	
public:
	UMSGA_EnemyGroggy();

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
	UFUNCTION()
	void OnEventReceivedCallback(FGameplayTag EventTag, FGameplayEventData EventData);
	
private:
	UPROPERTY()
	uint32 GroggyCountAcc = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayEffect", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UGameplayEffect> RecoveryEffectClass;
};
