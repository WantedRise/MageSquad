// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Projectile/Behaviors/MSProjectileBehaviorBase.h"
#include "MSProjectileBehavior_ChainBolt.generated.h"

UCLASS()
class MAGESQUAD_API UMSProjectileBehavior_ChainBolt : public UMSProjectileBehaviorBase
{
	GENERATED_BODY()

public:
	virtual void OnBegin_Implementation() override;
	virtual void OnTargetEnter_Implementation(AActor* Target, const FHitResult& HitResult) override;
	virtual void OnEnd_Implementation() override;
	virtual void ApplyCollisionRadius(AMSBaseProjectile* InOwner, const FProjectileRuntimeData& InRuntimeData) override;

private:
	void PerformChainStep();
	void HandleArrival();
	AActor* FindClosestTarget(const FVector& Origin, float SearchRadius) const;
	void ApplyDamageToTarget(AActor* Target, float DamageAmount);
	void EndChain();

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> ChainSourceActor;

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> LastHitActor;

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> PendingTarget;

	UPROPERTY(Transient)
	TSet<TWeakObjectPtr<AActor>> HitActors;

	UPROPERTY(Transient)
	int32 RemainingChains = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Chain")
	int32 MaxChains = 4;

	UPROPERTY(EditDefaultsOnly, Category = "Chain")
	float InitialChainRange = 1200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Chain")
	float ChainInterval = 0.1f;

	UPROPERTY(Transient)
	FTimerHandle ChainTimerHandle;

	UPROPERTY(Transient)
	FTimerHandle TravelTimerHandle;
};
