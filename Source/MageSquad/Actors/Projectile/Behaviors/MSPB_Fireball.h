// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Projectile/Behaviors/MSProjectileBehaviorBase.h"
#include "MSPB_Fireball.generated.h"

/**
 * Fireball split explosion behavior (second burst).
 */
UCLASS(BlueprintType, Blueprintable)
class MAGESQUAD_API UMSPB_Fireball : public UMSProjectileBehaviorBase
{
	GENERATED_BODY()

public:
	virtual void OnBegin_Implementation() override;

	virtual void OnTargetEnter_Implementation(
		AActor* Target,
		const FHitResult& HitResult
	) override;

	virtual void OnEnd_Implementation() override;

	virtual void ApplyCollisionRadius(
		AMSBaseProjectile* InOwner,
		const FProjectileRuntimeData& InRuntimeData
	) override;

protected:
	void ExplodeAt(const FVector& ExplosionOrigin);
	void ApplyDamageToTarget(AActor* Target, float DamageAmount);
	void HandleSecondExplosion();

private:
	bool bExploded = false;
	FVector SecondExplosionOrigin = FVector::ZeroVector;
	FTimerHandle SecondExplosionTimer;
};
