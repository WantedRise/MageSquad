// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Projectile/Behaviors/MSProjectileBehaviorBase.h"
#include "MSPB_TornadoEnhanced.generated.h"

class UAudioComponent;

/**
 * Enhanced Tornado: splits into two every interval, shrinking radius/scale each time.
 */
UCLASS()
class MAGESQUAD_API UMSPB_TornadoEnhanced : public UMSProjectileBehaviorBase
{
	GENERATED_BODY()
	
public:
	virtual void OnBegin_Implementation() override;
	virtual void OnEnd_Implementation() override;

private:
	void StartMove();
	void StopMove();
	void TickMove();

	void StartPeriodicDamage();
	void StopPeriodicDamage();
	void TickPeriodicDamage();
	void ApplyDamageToTarget(AActor* Target, float DamageAmount);

	void StartSplit();
	void StopSplit();
	void TickSplit();
	void SpawnSplitProjectile(const FVector& Direction, float RemainingLife, float NextRadiusScale);

private:
	FTimerHandle MoveTimerHandle;
	FTimerHandle DamageTimerHandle;
	FTimerHandle SplitTimerHandle;

	TWeakObjectPtr<UAudioComponent> LoopingSFX;

	FVector StartLocation = FVector::ZeroVector;
	FVector ForwardDir = FVector::ForwardVector;
	float StartTime = 0.f;

	float MoveSpeed = 500.f;
	float SwirlAmp = 120.f;
	float SwirlFreq = 7.f;
	float NoiseAmp = 60.f;
	float NoiseFreq = 1.3f;

	float SplitInterval = 2.f;
	float SplitRadiusScale = 0.9f;
	float SplitYawOffset = 30.f;

	bool bEnded = false;
};



