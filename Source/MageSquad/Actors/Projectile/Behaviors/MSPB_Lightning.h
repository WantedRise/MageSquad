// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Projectile/Behaviors/MSProjectileBehaviorBase.h"
#include "MSPB_Lightning.generated.h"

UCLASS()
class MAGESQUAD_API UMSPB_Lightning : public UMSProjectileBehaviorBase
{
	GENERATED_BODY()

public:
	virtual void OnBegin_Implementation() override;
	virtual void OnTargetEnter_Implementation(AActor* Target, const FHitResult& HitResult) override;
	virtual void OnEnd_Implementation() override;

private:
	void ApplyDamageInRadius(float Radius, float Damage, float VfxScale);
	void HandleDelayedStrike();

private:
	bool bDamageApplied = false;
	FVector ImpactOrigin = FVector::ZeroVector;
	FTimerHandle DelayedStrikeHandle;
};
