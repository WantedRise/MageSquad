// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Projectile/Behaviors/MSProjectileBehaviorBase.h"
#include "MSPB_Tornado.generated.h"

class UAudioComponent;

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UMSPB_Tornado : public UMSProjectileBehaviorBase
{
	GENERATED_BODY()
	
public:
	virtual void OnBegin_Implementation() override;
	virtual void OnEnd_Implementation() override;

private:
	// 이동
	void StartMove();
	void StopMove();
	void TickMove();

	// 대미지
	void StartPeriodicDamage();
	void StopPeriodicDamage();
	void TickPeriodicDamage();
	void ApplyDamageToTarget(AActor* Target, float DamageAmount);

private:
	// 루프 SFX 핸들
	TWeakObjectPtr<UAudioComponent> LoopingSFX;

	FTimerHandle MoveTimerHandle;
	FTimerHandle DamageTimerHandle;

	// 이동 베이스
	FVector StartLocation = FVector::ZeroVector;
	FVector ForwardDir = FVector::ForwardVector;
	float StartTime = 0.f;

	// 이동 파라미터
	float MoveSpeed = 500.f;     // 전진 속도
	float SwirlAmp = 120.f;      // 흔들림 크기(좌우)
	float SwirlFreq = 7.f;       // 흔들림 속도
	float NoiseAmp = 60.f;       // 불규칙성 크기
	float NoiseFreq = 1.3f;      // 불규칙성 변화 속도

	// 종료 중복 방지
	bool bEnded = false;
};
