// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Projectile/Behaviors/MSProjectileBehaviorBase.h"
#include "MSPB_AreaPeriodic.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UMSPB_AreaPeriodic : public UMSProjectileBehaviorBase
{
	GENERATED_BODY()
	

public:
	virtual void OnBegin_Implementation() override;
	virtual void OnEnd_Implementation() override;

private:
	void StartPeriodicDamage();
	void StopPeriodicDamage();

	/** 타이머 콜백: 한 틱 데미지 처리 */
	void TickPeriodicDamage();

	/** 단일 타겟에게 데미지 GE 적용 */
	void ApplyDamageToTarget(AActor* Target, float DamageAmount);

private:
	FTimerHandle PeriodicTimerHandle;

	int32 CurrentTickIndex = 0;
	bool bRunning = false;
};
