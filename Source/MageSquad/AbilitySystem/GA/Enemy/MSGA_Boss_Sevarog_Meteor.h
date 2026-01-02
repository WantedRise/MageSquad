// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GA/Enemy/MSGA_EnemyBossBaseAbility.h"
#include "Types/MSIndicatorTypes.h"
#include "MSGA_Boss_Sevarog_Meteor.generated.h"

/**
 * 작성자 : 임희섭
 * 작성일 : 2026/01/02
 * Boss Sevarog의 Pattern1 어빌리티
* 모든 플레이어를 추격하며 바닥에 폭발을 일으킨다.
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
	UFUNCTION()
	void OnChaseComplete();
	// Indicator 스폰 콜백 (사운드, VFX 등 추가 처리용)
	UFUNCTION()
	void OnIndicatorSpawned(AMSIndicatorActor* Indicator, FVector Location);
	
protected:
	// ===== Meteor 설정 =====

	// 스폰할 Indicator 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Meteor")
	TSubclassOf<AMSIndicatorActor> IndicatorActorClass;

	// 적용할 데미지 GameplayEffect
	UPROPERTY(EditDefaultsOnly, Category = "Meteor")
	TSubclassOf<UGameplayEffect> MeteorDamageEffect;

	// 추적 총 시간
	UPROPERTY(EditDefaultsOnly, Category = "Meteor")
	float ChaseDuration = 5.f;

	// Indicator 스폰 간격
	UPROPERTY(EditDefaultsOnly, Category = "Meteor")
	float SpawnInterval = 0.5f;

	// Indicator 파라미터 (Shape, Radius, Duration 등)
	UPROPERTY(EditDefaultsOnly, Category = "Meteor")
	FAttackIndicatorParams IndicatorParams;
	
};
