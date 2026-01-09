// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Projectile/Behaviors/MSProjectileBehaviorBase.h"
#include "MSPB_Explosive.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UMSPB_Explosive : public UMSProjectileBehaviorBase
{
	GENERATED_BODY()

public:
	virtual void OnBegin_Implementation() override;
	virtual void OnTargetEnter_Implementation(AActor* Target, const FHitResult& HitResult) override;
	virtual void OnEnd_Implementation() override;
	virtual void ApplyCollisionRadius(AMSBaseProjectile* InOwner, const FProjectileRuntimeData& InRuntimeData) override;

private:
	void ApplyDamageToTarget(AActor* Target, float DamageAmount);

private:
	UPROPERTY(Transient)
	TSet<TWeakObjectPtr<AActor>> HitActors;

	bool bExploded = false;
};
