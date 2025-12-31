// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Indicator/MSIndicatorActor.h"
#include "Components/DecalComponent.h"
#include "Math/UnrealMathUtility.h" // DegreesToRadians용 헤더
#include "Net/UnrealNetwork.h"

// Sets default values
AMSIndicatorActor::AMSIndicatorActor()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// 네트워크 세팅
	bReplicates = true;
	bAlwaysRelevant = true;
	
	DecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("Decal"));
	RootComponent = DecalComponent;
	
	// 바닥을 향하도록 기본 회전 설정
	DecalComponent->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
}

void AMSIndicatorActor::Initialize(const FAttackIndicatorParams& Params)
{
	CachedParams = Params;
	ElapsedTime = 0.f;
	ApplyMaterialParams();
}


// Called every frame
void AMSIndicatorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	ElapsedTime += DeltaTime;

	if (ElapsedTime >= CachedParams.Duration)
	{
		Destroy();
		return;
	}

	UpdateFillPercent();
}

void AMSIndicatorActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMSIndicatorActor, CachedParams);
}

void AMSIndicatorActor::OnRep_CachedParams()
{
	ElapsedTime = 0.f;
	ApplyMaterialParams();
}

void AMSIndicatorActor::ApplyMaterialParams()
{
	if (!IndicatorMaterial)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMSIndicatorActor: IndicatorMaterial is not set!"));
		return;
	}

	if (!DynamicMaterial)
	{
		DynamicMaterial = UMaterialInstanceDynamic::Create(IndicatorMaterial, this);
		DecalComponent->SetDecalMaterial(DynamicMaterial);
	}

	DynamicMaterial->SetScalarParameterValue(TEXT("ShapeType"), static_cast<float>(CachedParams.Shape));
	DynamicMaterial->SetScalarParameterValue(TEXT("FillPercent"), 0.f);

	switch (CachedParams.Shape)
	{
	case EIndicatorShape::Circle:
		DynamicMaterial->SetScalarParameterValue(TEXT("Angle"), PI * 2.f);
		DecalComponent->DecalSize = FVector(CachedParams.Radius);
		break;

	case EIndicatorShape::Cone:
		DynamicMaterial->SetScalarParameterValue(TEXT("Angle"), CachedParams.Angle);
		DecalComponent->DecalSize = FVector(CachedParams.Radius);
		break;

	case EIndicatorShape::Rectangle:
		DynamicMaterial->SetScalarParameterValue(TEXT("AspectRatio"), CachedParams.Length / CachedParams.Width);
		DecalComponent->DecalSize = FVector(CachedParams.Length * 0.5f, CachedParams.Width * 0.5f, 100.f);
		break;
	}
}

void AMSIndicatorActor::UpdateFillPercent() const
{
	if (DynamicMaterial)
	{
		const float Percent = FMath::Clamp(ElapsedTime / CachedParams.Duration, 0.f, 1.f);
		DynamicMaterial->SetScalarParameterValue(TEXT("FillPercent"), Percent);
	}
}
