// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Types/MSIndicatorTypes.h"
#include "MSAT_ChaseAndSpawnMeteor.generated.h"

class AMSIndicatorActor;
/*
 * 작성자 : 임희섭
 * 작성일 : 2026/01/02
 * 플레이어를 추적하며 일정 간격으로 Meteor Indicator를 스폰하는 AbilityTask
 * 모든 플레이어를 대상으로 스폰
 */
UCLASS()
class MAGESQUAD_API UMSAT_ChaseAndSpawnMeteor : public UAbilityTask
{
	GENERATED_BODY()
	
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnChaseComplete);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnIndicatorSpawned, AMSIndicatorActor*, Indicator, FVector, Location);

	// 추적 완료 시 호출
	UPROPERTY(BlueprintAssignable)
	FOnChaseComplete OnChaseComplete;

	// Indicator 스폰 시마다 호출 (사운드, 추가 이펙트 등에 활용)
	UPROPERTY(BlueprintAssignable)
	FOnIndicatorSpawned OnIndicatorSpawned;
	
	/**
 * Task 생성 함수
 * @param OwningAbility - 소유 Ability
 * @param InTotalDuration - 총 추적 시간
 * @param InSpawnInterval - Indicator 스폰 간격
 * @param IndicatorClass - 스폰할 Indicator 클래스
 * @param IndicatorParams - Indicator 초기화 파라미터
 * @param DamageEffect - 적용할 데미지 GameplayEffect (선택)
 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks",
		meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
	static UMSAT_ChaseAndSpawnMeteor* CreateTask(
		UGameplayAbility* OwningAbility,
		float InTotalDuration,
		float InSpawnInterval,
		TSubclassOf<AMSIndicatorActor> IndicatorClass,
		const FAttackIndicatorParams& IndicatorParams,
		TSubclassOf<UGameplayEffect> DamageEffect = nullptr);

protected:
	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;
	virtual void OnDestroy(bool bInOwnerFinished) override;

private:
	// 모든 플레이어 위치에 Indicator 스폰
	void SpawnIndicatorsOnAllPlayers();
	
	// 특정 위치에 Indicator 스폰
	AMSIndicatorActor* SpawnIndicatorAtLocation(const FVector& Location);
	
	// 바닥 높이 계산 (LineTrace)
	float GetGroundZ(const FVector& Location) const;

	// 설정값
	float TotalDuration = 5.f;
	float SpawnInterval = 0.5f;
	
	UPROPERTY()
	TSubclassOf<AMSIndicatorActor> IndicatorActorClass;
	
	UPROPERTY()
	TSubclassOf<UGameplayEffect> DamageEffectClass;
	
	FAttackIndicatorParams CachedIndicatorParams;

	// 런타임
	float ElapsedTime = 0.f;
	float TimeSinceLastSpawn = 0.f;
	
};
