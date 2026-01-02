// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/TargetActor/MSTargetActor_Cone.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"

AMSTargetActor_Cone::AMSTargetActor_Cone()
{
}

TArray<AActor*> AMSTargetActor_Cone::PerformOverlapCheck()
{
	TArray<AActor*> HitActors;
	TArray<FOverlapResult> Overlaps;

	const FVector OverlapLocation = GetActorLocation();
	const float Radius = CachedParams.Radius;

	// 1단계: Sphere로 대략적인 범위 내 액터 수집
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
		// 2단계: Cone 각도 내에 있는지 필터링
		for (const FOverlapResult& Overlap : Overlaps)
		{
			AActor* HitActor = Overlap.GetActor();
			if (IsValidTarget(HitActor) && IsActorInCone(HitActor))
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

bool AMSTargetActor_Cone::IsActorInCone(const AActor* Actor) const
{
	if (!Actor)
	{
		return false;
	}

	const FVector ConeOrigin = GetActorLocation();
	const FVector ActorLocation = Actor->GetActorLocation();
	const FVector ToActor = ActorLocation - ConeOrigin;

	// 거리 체크 (2D)
	const float DistanceSq2D = ToActor.SizeSquared2D();
	if (DistanceSq2D > FMath::Square(CachedParams.Radius))
	{
		return false;
	}

	// 거리가 0에 가까우면 (원점에 있으면) 무조건 포함
	if (DistanceSq2D < KINDA_SMALL_NUMBER)
	{
		return true;
	}

	// 각도 체크 (2D - XY 평면)
	const FVector ForwardDir = GetActorForwardVector().GetSafeNormal2D();
	const FVector ToActorDir = ToActor.GetSafeNormal2D();

	// 두 벡터 사이의 각도 계산
	const float DotProduct = FVector::DotProduct(ForwardDir, ToActorDir);
	const float AngleToActorRad = FMath::Acos(FMath::Clamp(DotProduct, -1.f, 1.f));

	// CachedParams.Angle은 전체 부채꼴 각도(도)이므로 절반을 라디안으로 변환하여 비교
	const float HalfAngleRad = FMath::DegreesToRadians(CachedParams.Angle * 0.5f);

	return AngleToActorRad <= HalfAngleRad;
}

void AMSTargetActor_Cone::DrawDebugTargetArea() const
{
#if WITH_EDITOR
	const FVector Location = GetActorLocation();
	const FVector ForwardDir = GetActorForwardVector();
	const float Radius = CachedParams.Radius;
	const float HalfAngleDeg = CachedParams.Angle * 0.5f;

	// Cone의 두 끝선 그리기
	const FVector LeftDir = ForwardDir.RotateAngleAxis(-HalfAngleDeg, FVector::UpVector);
	const FVector RightDir = ForwardDir.RotateAngleAxis(HalfAngleDeg, FVector::UpVector);

	const FVector LeftEnd = Location + LeftDir * Radius;
	const FVector RightEnd = Location + RightDir * Radius;
	const FVector ForwardEnd = Location + ForwardDir * Radius;

	// 원점에서 좌/우 끝점까지 선
	DrawDebugLine(GetWorld(), Location, LeftEnd, FColor::Red, false, DebugDrawDuration, 0, 2.f);
	DrawDebugLine(GetWorld(), Location, RightEnd, FColor::Red, false, DebugDrawDuration, 0, 2.f);
	DrawDebugLine(GetWorld(), Location, ForwardEnd, FColor::Yellow, false, DebugDrawDuration, 0, 2.f);

	// 호(Arc) 그리기 - 여러 세그먼트로 근사
	const int32 NumSegments = FMath::Max(3, static_cast<int32>(CachedParams.Angle / 10.f));
	const float AngleStep = CachedParams.Angle / NumSegments;

	FVector PrevPoint = LeftEnd;
	for (int32 i = 1; i <= NumSegments; ++i)
	{
		const float CurrentAngle = -HalfAngleDeg + (AngleStep * i);
		const FVector CurrentDir = ForwardDir.RotateAngleAxis(CurrentAngle, FVector::UpVector);
		const FVector CurrentPoint = Location + CurrentDir * Radius;

		DrawDebugLine(GetWorld(), PrevPoint, CurrentPoint, FColor::Red, false, DebugDrawDuration, 0, 2.f);
		PrevPoint = CurrentPoint;
	}

	// 중심점 표시
	DrawDebugPoint(GetWorld(), Location, 10.f, FColor::Green, false, DebugDrawDuration);
#endif
}
