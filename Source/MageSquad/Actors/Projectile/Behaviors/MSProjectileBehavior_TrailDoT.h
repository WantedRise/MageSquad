// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Projectile/Behaviors/MSProjectileBehaviorBase.h"
#include "MSProjectileBehavior_TrailDoT.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UMSProjectileBehavior_TrailDoT : public UMSProjectileBehaviorBase
{
	GENERATED_BODY()

public:
	virtual void OnBegin_Implementation() override;
	virtual void OnTargetEnter_Implementation(AActor* Target, const FHitResult& HitResult) override;
	virtual void OnEnd_Implementation() override;
	virtual void ApplyCollisionRadius(AMSBaseProjectile* InOwner, const FProjectileRuntimeData& InRuntimeData) override;

private:
	void SampleTrailPoint();
	void TickTrailDamage();
	void EndTrail();
	void HandleDirectHitAtPoint(const FVector& Point);
	bool CanHitDirectTarget(AActor* Target) const;
	void ApplyDamageToTarget(AActor* Target, float DamageAmount);

private:
	UPROPERTY(EditDefaultsOnly, Category = "Trail")
	float TrailSampleInterval = 0.05f;

	UPROPERTY(EditDefaultsOnly, Category = "Trail")
	float DefaultTrailLifetime = 2.f;

	UPROPERTY(Transient)
	TArray<FVector> TrailPoints;

	UPROPERTY(Transient)
	TSet<TWeakObjectPtr<AActor>> DirectHitActors;

	UPROPERTY(Transient)
	FVector StartLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	FVector VirtualLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	FVector TravelDirection = FVector::ForwardVector;

	UPROPERTY(Transient)
	bool bMovementStopped = false;

	UPROPERTY(Transient)
	FTimerHandle TrailSampleTimerHandle;

	UPROPERTY(Transient)
	FTimerHandle TrailDamageTimerHandle;

	UPROPERTY(Transient)
	FTimerHandle TrailEndTimerHandle;
};
