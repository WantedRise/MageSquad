// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GA/Enemy/MSGA_EnemyBaseAbility.h"
#include "MSGA_EnemyDead.generated.h"

/**
 * 작성자 : 임희섭
 * 작성일 : 2025/12/17
 * Enemy들이 죽을 때 발동될 Dead Ability
 */
UCLASS()
class MAGESQUAD_API UMSGA_EnemyDead : public UMSGA_EnemyBaseAbility
{
	GENERATED_BODY()

public:
	UMSGA_EnemyDead();

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

private:
	/*
	* 수정자: 김준형
	* 수정일: 26/01/09
	* 
	* 적 사망 시 아이템 드롭 구현
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AMSExperienceOrb> ExpReward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AMSMagnetOrb> MagnetReward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AMSPotionOrb> PotionReward;

	// 아이템 중복 드롭 방지 플래그
	bool bHasDroppedItem = false;
	
	bool bEndAbilityCalled = false;
};
