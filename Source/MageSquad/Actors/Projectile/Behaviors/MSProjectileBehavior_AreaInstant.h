// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Projectile/Behaviors/MSProjectileBehaviorBase.h"
#include "MSProjectileBehavior_AreaInstant.generated.h"

/**
 * 작성자: 박세찬
 * 작성일: 26/01/01
 * 
 * 일정 범위에 있는 몬스터에게 데미지를 1번 줌
 */
UCLASS()
class MAGESQUAD_API UMSProjectileBehavior_AreaInstant : public UMSProjectileBehaviorBase
{
	GENERATED_BODY()
public:
	virtual void OnBegin_Implementation() override;
	virtual void OnTargetEnter_Implementation(AActor* Target, const FHitResult& HitResult) override;
	virtual void OnEnd_Implementation() override;

private:
	void ApplyDamageToTarget(AActor* Target, float DamageAmount);

private:
	UPROPERTY(Transient)
	TSet<TWeakObjectPtr<AActor>> HitActors;
	
	bool bDamageApplied = false;
};
