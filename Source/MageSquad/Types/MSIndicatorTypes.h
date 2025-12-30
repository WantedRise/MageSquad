// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MSIndicatorTypes.generated.h"

/*
 * 작성자 : 임희섭
 * 작성일 : 2025/12/30
 * 보스 패턴의 인디케이터 지정을 위한 데이터 구조체
 */

UENUM(BlueprintType)
enum class EIndicatorShape : uint8
{
	Circle,      // 원형 (360도)
	Cone,        // 부채꼴
	Rectangle    // 사각형
};

USTRUCT(BlueprintType)
struct MAGESQUAD_API  FAttackIndicatorParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EIndicatorShape Shape = EIndicatorShape::Circle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 1.5f;

	// 원형/부채꼴용
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "Shape != EIndicatorShape::Rectangle"))
	float Radius = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "Shape == EIndicatorShape::Cone"))
	float Angle = 90.f;  // 부채꼴 각도

	// 사각형용
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "Shape == EIndicatorShape::Rectangle"))
	float Width = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "Shape == EIndicatorShape::Rectangle"))
	float Length = 500.f;
};