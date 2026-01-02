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

    // 디버깅용: 파라미터 확인
    // UE_LOG(LogTemp, Log, TEXT("Cone Check: Loc=%s, Radius=%f, Angle=%f, Channel=%d"), 
    //     *OverlapLocation.ToString(), Radius, CachedParams.Angle, TargetCollisionChannel.GetValue());

    FCollisionShape SphereShape = FCollisionShape::MakeSphere(Radius);
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.AddIgnoredActor(GetOwner());

    // 1단계: Sphere Overlap
    bool bOverlapFound = GetWorld()->OverlapMultiByChannel(
        Overlaps,
        OverlapLocation,
        FQuat::Identity,
        TargetCollisionChannel,
        SphereShape,
        QueryParams
    );

    if (bOverlapFound)
    {
        // 2단계: Cone 필터링
        for (const FOverlapResult& Overlap : Overlaps)
        {
            AActor* HitActor = Overlap.GetActor();
            
            // 유효성 검사 (BaseClass)
            if (!IsValidTarget(HitActor)) continue;

            // 각도 검사
            if (IsActorInCone(HitActor))
            {
                HitActors.AddUnique(HitActor);
            }
        }
    }

#if WITH_EDITOR
    if (bDrawDebug)
    {
        // 하나라도 잡혔으면 Green, 아니면 Red
        DrawDebugTargetArea(!HitActors.IsEmpty());
    }
#endif

    return HitActors;
}

bool AMSTargetActor_Cone::IsActorInCone(const AActor* Actor) const
{
    if (!Actor) return false;

    const FVector ConeOrigin = GetActorLocation();
    const FVector ActorLocation = Actor->GetActorLocation();
    const FVector ToActor = ActorLocation - ConeOrigin;

    // 거리 체크 (2D)
    const float DistanceSq2D = ToActor.SizeSquared2D();
    if (DistanceSq2D > FMath::Square(CachedParams.Radius)) return false;
    if (DistanceSq2D < KINDA_SMALL_NUMBER) return true;

    // 회전 문제 방지: 액터의 3D Forward가 아니라, Yaw만 반영된 2D Forward를 강제로 구함
    FRotator Rotation = GetActorRotation();
    Rotation.Pitch = 0.f; // Pitch 강제 0
    Rotation.Roll = 0.f;  // Roll 강제 0
    const FVector ForwardDir = Rotation.Vector().GetSafeNormal2D();
    
    const FVector ToActorDir = ToActor.GetSafeNormal2D();

    const float DotProduct = FVector::DotProduct(ForwardDir, ToActorDir);
    const float AngleToActorRad = FMath::Acos(FMath::Clamp(DotProduct, -1.f, 1.f));
    const float HalfAngleRad = FMath::DegreesToRadians(CachedParams.Angle * 0.5f);

    return AngleToActorRad <= HalfAngleRad;
}

void AMSTargetActor_Cone::DrawDebugTargetArea(bool bHasHit) const
{
#if WITH_EDITOR
    const FVector Location = GetActorLocation();
    const float Radius = CachedParams.Radius;
    const float HalfAngleDeg = CachedParams.Angle * 0.5f;
    
    // [중요] 디버그 드로잉도 실제 계산과 동일하게 Pitch를 무시해야 정확한 범위가 보임
    FRotator Rotation = GetActorRotation();
    Rotation.Pitch = 0.f;
    const FVector ForwardDir = Rotation.Vector();

    // Cone의 두 끝선 그리기
    const FVector LeftDir = ForwardDir.RotateAngleAxis(-HalfAngleDeg, FVector::UpVector);
    const FVector RightDir = ForwardDir.RotateAngleAxis(HalfAngleDeg, FVector::UpVector);

    const FVector LeftEnd = Location + LeftDir * Radius;
    const FVector RightEnd = Location + RightDir * Radius;
    const FVector ForwardEnd = Location + ForwardDir * Radius;

    // 색상 결정 (삼항 연산자)
    FColor DebugColor = bHasHit ? FColor::Green : FColor::Red;

    DrawDebugLine(GetWorld(), Location, LeftEnd, DebugColor, false, DebugDrawDuration, 0, 2.f);
    DrawDebugLine(GetWorld(), Location, RightEnd, DebugColor, false, DebugDrawDuration, 0, 2.f);
    DrawDebugLine(GetWorld(), Location, ForwardEnd, FColor::Yellow, false, DebugDrawDuration, 0, 2.f);

    // 호 그리기
    const int32 NumSegments = FMath::Max(3, static_cast<int32>(CachedParams.Angle / 10.f));
    const float AngleStep = CachedParams.Angle / NumSegments;

    FVector PrevPoint = LeftEnd;
    for (int32 i = 1; i <= NumSegments; ++i)
    {
        const float CurrentAngle = -HalfAngleDeg + (AngleStep * i);
        const FVector CurrentDir = ForwardDir.RotateAngleAxis(CurrentAngle, FVector::UpVector);
        const FVector CurrentPoint = Location + CurrentDir * Radius;

        DrawDebugLine(GetWorld(), PrevPoint, CurrentPoint, DebugColor, false, DebugDrawDuration, 0, 2.f);
        PrevPoint = CurrentPoint;
    }

    DrawDebugPoint(GetWorld(), Location, 10.f, DebugColor, false, DebugDrawDuration);
#endif
}