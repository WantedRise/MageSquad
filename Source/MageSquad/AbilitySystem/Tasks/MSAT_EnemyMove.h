// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "MSAT_EnemyMove.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UMSAT_EnemyMove : public UAbilityTask
{
	GENERATED_BODY()
	
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMoveTaskDelegate);
    
	UPROPERTY(BlueprintAssignable)
	FOnMoveTaskDelegate OnTargetReached;
    
	UPROPERTY(BlueprintAssignable)
	FOnMoveTaskDelegate OnTargetLost;

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", 
		meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility"))
	static UMSAT_EnemyMove* CreateTask(
		UGameplayAbility* OwningAbility,
		float AcceptanceRadius = 5.f,
		float TargetUpdateInterval = 0.25f);

	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;
	virtual void OnDestroy(bool bInOwnerFinished) override;

private:
	void UpdateMoveTarget();
	
private:
	UPROPERTY()
	TWeakObjectPtr<class AMSBaseAIController> CachedAIC;
    
	UPROPERTY()
	TWeakObjectPtr<AActor> CachedTargetActor;

	float AcceptanceRadius;
	float UpdateInterval;
	float TimeSinceLastUpdate;
    
	// 최적화: 이전 타겟 위치 캐싱 (불필요한 MoveToActor 호출 방지)
	FVector LastTargetLocation;
	float TargetLocationThresholdSq;
};
