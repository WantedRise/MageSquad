// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Projectile/Behaviors/MSProjectileBehaviorBase.h"
#include "MSPB_Normal.generated.h"

/**
 * 정석적인 단일 투사체 행동
 * - RuntimeData.PenetrationCount 기반 관통
 * - 중복 히트 방지
 * - 소유자 / 인스티게이터 무시
 * - 기본: 관통 횟수 소진 시 파괴
 */
UCLASS(BlueprintType, Blueprintable)
class MAGESQUAD_API UMSPB_Normal : public UMSProjectileBehaviorBase
{
	GENERATED_BODY()

public:
	/** 시작 시 초기화 */
	virtual void OnBegin_Implementation() override;

	/** 타겟 히트 처리 */
	virtual void OnTargetEnter_Implementation(
		AActor* Target,
		const FHitResult& HitResult
	) override;

	/** 종료 시 정리 */
	virtual void OnEnd_Implementation() override;

protected:
	/** 히트 가능 여부 판별 */
	bool CanHitTarget(AActor* Target) const;

	/** 실제 히트 처리 (데미지/GE는 나중에 추가) */
	void HandleHitTarget(AActor* Target, const FHitResult& HitResult);

protected:
	/** 이미 맞은 액터 목록 (중복 히트 방지) */
	UPROPERTY(Transient)
	TSet<TWeakObjectPtr<AActor>> HitActors;

private:
	/** 남은 관통 횟수 (RuntimeData 기반) */
	int32 RemainingPenetration = 0;
};

