// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Projectile/Behaviors/MSProjectileBehaviorBase.h"
#include "MSPB_IceSpear.generated.h"

/**
 * IceSpear split behavior.
 */
UCLASS(BlueprintType, Blueprintable)
class MAGESQUAD_API UMSPB_IceSpear : public UMSProjectileBehaviorBase
{
	GENERATED_BODY()

public:
	virtual void OnBegin_Implementation() override;

	virtual void OnTargetEnter_Implementation(
		AActor* Target,
		const FHitResult& HitResult
	) override;

	virtual void OnEnd_Implementation() override;

protected:
	bool CanHitTarget(AActor* Target) const;
	void HandleHitTarget(AActor* Target, const FHitResult& HitResult);

	bool TrySplitOnFirstHit(AActor* HitTarget, const FHitResult& HitResult);
	bool SpawnSplitProjectileDir(const FVector& Dir, const FVector& Origin, int32 NextPenetration, AActor* IgnoreActor, bool bReplicate);

protected:
	UPROPERTY(Transient)
	TSet<TWeakObjectPtr<AActor>> HitActors;

private:
	int32 RemainingPenetration = 0;
	bool bHasSplit = false;
};
