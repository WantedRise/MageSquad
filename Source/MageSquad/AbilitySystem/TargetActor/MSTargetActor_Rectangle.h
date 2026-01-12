// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/TargetActor/MSTargetActor_IndicatorBase.h"
#include "MSTargetActor_Rectangle.generated.h"


/*
 * 작성자 : 임희섭
 * 작성일 : 2026/01/02
 * Rectangle Indicator와 연동하는 사각형 형태의 TargetActor
 * Indicator의 Width, Length를 사용하여 Box 충돌 검사 수행
 * 액터의 회전을 고려하여 로컬 좌표계에서 판정
 */
UCLASS()
class MAGESQUAD_API AMSTargetActor_Rectangle : public AMSTargetActor_IndicatorBase
{
	GENERATED_BODY()

public:
	AMSTargetActor_Rectangle();

protected:
	virtual TArray<AActor*> PerformOverlapCheck() override;

private:
	// 액터가 Rectangle 범위 내에 있는지 정밀 판정
	bool IsActorInRectangle(const AActor* Actor) const;

	// 디버그용 Rectangle 그리기
	void DrawDebugTargetArea(bool IsValid) const;
};
