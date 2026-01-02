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

	// const FVector OverlapLocation = GetActorLocation();
	// const FQuat OverlapRotation = GetActorQuat();
	
	// Box Extent 계산 (Width = Y축, Length = X축)
	const float HalfLength = CachedParams.Length * 0.5f;
	const float HalfWidth = CachedParams.Width * 0.5f;
	const FVector BoxExtent(HalfLength, HalfWidth, 100.f); // Z는 충분한 높이
	
	// 박스의 중심을 전방으로 HalfLength만큼 밀어줌
	//const FVector OverlapLocation = GetActorLocation() + (GetActorForwardVector() * HalfLength);
	const FVector OverlapLocation = GetActorLocation();
	const FQuat OverlapRotation = GetActorQuat();

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
		DrawDebugTargetArea(!HitActors.IsEmpty());
	}
#endif

	return HitActors;
}

bool AMSTargetActor_Rectangle::IsActorInRectangle(const AActor* Actor) const
{
	if (!Actor)
	{
		return false;
	}
	
	// 액터 기준 로컬 좌표로 변환
	const FVector LocalPos = GetActorTransform().InverseTransformPosition(Actor->GetActorLocation());
	
	// [수정] X는 0(발밑) ~ Length(정면 끝), Y는 -HalfWidth ~ +HalfWidth
	return (LocalPos.X >= 0.f && LocalPos.X <= CachedParams.Length) && 
		   (FMath::Abs(LocalPos.Y) <= (CachedParams.Width * 0.5f));
}

void AMSTargetActor_Rectangle::DrawDebugTargetArea(bool IsValid) const
{
#if WITH_EDITOR
	const FVector Location = GetActorLocation();
	const FRotator Rotation = GetActorRotation();
	const FVector ForwardDir = GetActorForwardVector();

	const float HalfLength = CachedParams.Length * 0.5f;
	const float HalfWidth = CachedParams.Width * 0.5f;
	
	const FVector BoxExtent(HalfLength, HalfWidth, 100.f); // 높이는 시인성을 위해 조정

	DrawDebugBox(
	   GetWorld(),
	   //DebugBoxCenter, // Location 대신 밀어준 중심점 사용
	   Location,
	   BoxExtent,
	   FQuat(Rotation),
	   IsValid == true ? FColor::Green : FColor::Red,
	   false,
	   DebugDrawDuration,
	   0,
	   2.f);

#endif

}
