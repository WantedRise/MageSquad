// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/TargetActor/MSTargetActor_IndicatorBase.h"
#include "MSTargetActor_Sphere.generated.h"

/*
 * 작성자 : 임희섭
 * 작성일 : 2026/01/02
 * Circle Indicator와 연동하는 Sphere 형태의 TargetActor
 * Indicator의 Radius를 사용하여 구체 충돌 검사 수행
 */
UCLASS()
class MAGESQUAD_API AMSTargetActor_Sphere : public AMSTargetActor_IndicatorBase
{
	GENERATED_BODY()

public:
	AMSTargetActor_Sphere();

protected:
	virtual TArray<AActor*> PerformOverlapCheck() override;

private:
	// 디버그용 Sphere 그리기
	void DrawDebugTargetArea(bool IsValid) const;
};
