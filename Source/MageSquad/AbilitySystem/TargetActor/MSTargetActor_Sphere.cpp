// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/TargetActor/MSTargetActor_Sphere.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"

AMSTargetActor_Sphere::AMSTargetActor_Sphere()
{
}

TArray<AActor*> AMSTargetActor_Sphere::PerformOverlapCheck()
{
	TArray<AActor*> HitActors;
	TArray<FOverlapResult> Overlaps;

	const FVector OverlapLocation = GetActorLocation();
	const float Radius = CachedParams.Radius;

	FCollisionShape SphereShape = FCollisionShape::MakeSphere(Radius);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(GetOwner());

	if (GetWorld()->OverlapMultiByChannel(
		Overlaps,
		OverlapLocation,
		FQuat::Identity,
		TargetCollisionChannel,
		SphereShape,
		QueryParams))
	{
		for (const FOverlapResult& Overlap : Overlaps)
		{
			AActor* HitActor = Overlap.GetActor();
			if (IsValidTarget(HitActor))
			{
				HitActors.AddUnique(HitActor);
			}
		}
	}

#if WITH_EDITOR
	if (bDrawDebug)
	{
		DrawDebugTargetArea();
	}
#endif

	return HitActors;
}

void AMSTargetActor_Sphere::DrawDebugTargetArea() const
{
#if WITH_EDITOR
	const FVector Location = GetActorLocation();
	const float Radius = CachedParams.Radius;

	DrawDebugSphere(
		GetWorld(),
		Location,
		Radius,
		24,
		FColor::Red,
		false,
		DebugDrawDuration,
		0,
		2.f);
#endif
}
