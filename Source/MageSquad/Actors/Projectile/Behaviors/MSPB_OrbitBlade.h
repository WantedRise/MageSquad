// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Projectile/Behaviors/MSProjectileBehaviorBase.h"
#include "MSPB_OrbitBlade.generated.h"

UCLASS()
class MAGESQUAD_API UMSPB_OrbitBlade : public UMSProjectileBehaviorBase
{
	GENERATED_BODY()

public:
	virtual void OnBegin_Implementation() override;
	virtual void OnTargetEnter_Implementation(AActor* Target, const FHitResult& HitResult) override;
	virtual void OnEnd_Implementation() override;
	virtual void ApplyCollisionRadius(AMSBaseProjectile* InOwner, const FProjectileRuntimeData& InRuntimeData) override;

private:
	void TickOrbit();
	void EndOrbit();
	void ApplyDamageToTarget(AActor* Target, float DamageAmount);
	void ApplyVfxScale(float Scale);

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> OrbitCenter;

	UPROPERTY(Transient)
	float CurrentAngleRad = 0.f;

	UPROPERTY(Transient)
	float AngularSpeedRad = 0.f;

	UPROPERTY(Transient)
	float OrbitRadius = 0.f;

	UPROPERTY(Transient)
	FTimerHandle OrbitTimerHandle;

	UPROPERTY(Transient)
	FTimerHandle OrbitEndTimerHandle;
};
