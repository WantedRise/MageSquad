// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/TargetActor/MSTargetActor_Rectangle.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"

AMSTargetActor_Rectangle::AMSTargetActor_Rectangle()
{
}

TArray<AActor*> AMSTargetActor_Rectangle::PerformOverlapCheck()
{
	TArray<AActor*> HitActors;
	TArray<FOverlapResult> Overlaps;

	const FVector OverlapLocation = GetActorLocation();
	const FQuat OverlapRotation = GetActorQuat();

	// Box Extent 계산 (Width = Y축, Length = X축)
	const float HalfLength = CachedParams.Length * 0.5f;
	const float HalfWidth = CachedParams.Width * 0.5f;
	const FVector BoxExtent(HalfLength, HalfWidth, 100.f); // Z는 충분한 높이

	FCollisionShape BoxShape = FCollisionShape::MakeBox(BoxExtent);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(GetOwner());

	if (GetWorld()->OverlapMultiByChannel(
		Overlaps,
		OverlapLocation,
		OverlapRotation,
		TargetCollisionChannel,
		BoxShape,
		QueryParams))
	{
		for (const FOverlapResult& Overlap : Overlaps)
		{
			AActor* HitActor = Overlap.GetActor();
			if (IsValidTarget(HitActor) && IsActorInRectangle(HitActor))
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

bool AMSTargetActor_Rectangle::IsActorInRectangle(AActor* Actor) const
{
	if (!Actor)
	{
		return false;
	}

	// 월드 좌표를 로컬 좌표로 변환
	const FVector ActorWorldLocation = Actor->GetActorLocation();
	const FVector LocalPos = GetActorTransform().InverseTransformPosition(ActorWorldLocation);

	// Rectangle 범위 체크 (중심 기준, X = Length, Y = Width)
	const float HalfLength = CachedParams.Length * 0.5f;
	const float HalfWidth = CachedParams.Width * 0.5f;

	// XY 평면에서만 체크 (Z는 무시)
	return FMath::Abs(LocalPos.X) <= HalfLength && FMath::Abs(LocalPos.Y) <= HalfWidth;
}

void AMSTargetActor_Rectangle::DrawDebugTargetArea() const
{
#if WITH_EDITOR
	const FVector Location = GetActorLocation();
	const FRotator Rotation = GetActorRotation();

	const float HalfLength = CachedParams.Length * 0.5f;
	const float HalfWidth = CachedParams.Width * 0.5f;
	const FVector BoxExtent(HalfLength, HalfWidth, 5.f);

	DrawDebugBox(
		GetWorld(),
		Location,
		BoxExtent,
		FQuat(Rotation),
		FColor::Red,
		false,
		DebugDrawDuration,
		0,
		2.f);

	// 전방 방향 표시
	const FVector ForwardDir = GetActorForwardVector();
	DrawDebugDirectionalArrow(
		GetWorld(),
		Location,
		Location + ForwardDir * HalfLength,
		50.f,
		FColor::Yellow,
		false,
		DebugDrawDuration,
		0,
		2.f);
#endif
}
