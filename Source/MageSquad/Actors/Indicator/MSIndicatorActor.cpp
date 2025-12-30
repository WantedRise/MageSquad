// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Indicator/MSIndicatorActor.h"
#include "Components/DecalComponent.h"
#include "Math/UnrealMathUtility.h" // DegreesToRadians용 헤더

// Sets default values
AMSIndicatorActor::AMSIndicatorActor()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// 네트워크 세팅
	bReplicates = true;
	bAlwaysRelevant = true;

}

void AMSIndicatorActor::Initialize(const FAttackIndicatorParams& Params)
{
	CachedParams = Params;
	ElapsedTime = 0.f;

	// Dynamic Material 생성
	DynamicMaterial = DecalComponent->CreateDynamicMaterialInstance();

	// 모양에 따른 파라미터 설정
	DynamicMaterial->SetScalarParameterValue(TEXT("ShapeType"), static_cast<float>(Params.Shape));

	switch (Params.Shape)
	{
	case EIndicatorShape::Circle:
		DynamicMaterial->SetScalarParameterValue(TEXT("Angle"), PI * 2.f);
		DecalComponent->DecalSize = FVector(Params.Radius);
		break;

	case EIndicatorShape::Cone:
		DynamicMaterial->SetScalarParameterValue(TEXT("Angle"), FMath::DegreesToRadians(Params.Angle));
		DecalComponent->DecalSize = FVector(Params.Radius);
		break;

	case EIndicatorShape::Rectangle:
		DynamicMaterial->SetScalarParameterValue(TEXT("AspectRatio"), Params.Length / Params.Width);
		DecalComponent->DecalSize = FVector(Params.Length * 0.5f, Params.Width * 0.5f, 100.f);
		break;
	}

	DynamicMaterial->SetScalarParameterValue(TEXT("FillPercent"), 0.f);
}


// Called every frame
void AMSIndicatorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	ElapsedTime += DeltaTime;
	UpdateFillPercent();

	if (ElapsedTime >= CachedParams.Duration)
	{
		Destroy();
	}

}

void AMSIndicatorActor::UpdateFillPercent() const
{
	const float Percent = FMath::Clamp(ElapsedTime / CachedParams.Duration, 0.f, 1.f);
	DynamicMaterial->SetScalarParameterValue(TEXT("FillPercent"), Percent);
}

