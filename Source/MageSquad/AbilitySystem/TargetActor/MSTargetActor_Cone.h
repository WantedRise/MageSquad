// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/TargetActor/MSTargetActor_IndicatorBase.h"
#include "MSTargetActor_Cone.generated.h"

/*
 * 작성자 : 임희섭
 * 작성일 : 2026/01/02
 * Cone Indicator와 연동하는 부채꼴 형태의 TargetActor
 * 1단계: Sphere로 대략적인 범위 내 액터 수집
 * 2단계: Indicator의 Angle로 부채꼴 범위 내 액터 필터링
 */
UCLASS()
class MAGESQUAD_API AMSTargetActor_Cone : public AMSTargetActor_IndicatorBase
{
	GENERATED_BODY()

public:
	AMSTargetActor_Cone();

protected:
	virtual TArray<AActor*> PerformOverlapCheck() override;

private:
	// 액터가 Cone 범위 내에 있는지 판정
	bool IsActorInCone(const AActor* Actor) const;

	// 디버그용 Cone 그리기
	void DrawDebugTargetArea(bool bHasHit) const;
};
