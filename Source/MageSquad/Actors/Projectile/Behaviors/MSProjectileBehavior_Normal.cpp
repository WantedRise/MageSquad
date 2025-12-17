// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Projectile/Behaviors/MSProjectileBehavior_Normal.h"
#include "Actors/Projectile/MSBaseProjectile.h"
#include "GameFramework/Actor.h"

void UMSProjectileBehavior_Normal::OnBegin_Implementation()
{
	HitActors.Reset();

	// RuntimeData 기반 관통 횟수 초기화
	RemainingPenetration = FMath::Max(0, RuntimeData.PenetrationCount);
}

void UMSProjectileBehavior_Normal::OnTargetEnter_Implementation(
	AActor* Target,
	const FHitResult& HitResult
)
{
	// 서버에서만 피격 처리
	if (!IsAuthority())
	{
		return;
	}

	if (!CanHitTarget(Target))
	{
		return;
	}

	HandleHitTarget(Target, HitResult);

	AMSBaseProjectile* OwnerActor = GetOwnerActor();
	if (!OwnerActor)
	{
		return;
	}

	// 관통 횟수가 없으면 파괴
	if (RemainingPenetration <= 0)
	{
		OwnerActor->Destroy();
		return;
	}

	// 관통 횟수 소모
	RemainingPenetration--;
}

void UMSProjectileBehavior_Normal::OnEnd_Implementation()
{
	HitActors.Reset();
}

bool UMSProjectileBehavior_Normal::CanHitTarget(AActor* Target) const
{
	if (!IsValid(Target))
	{
		return false;
	}

	const AMSBaseProjectile* OwnerActor = GetOwnerActor();
	if (!OwnerActor)
	{
		return false;
	}

	// 자기 자신 무시
	if (Target == OwnerActor)
	{
		return false;
	}

	// 소유자 / 인스티게이터 무시
	if (Target == OwnerActor->GetOwner())
	{
		return false;
	}

	if (APawn* Inst = OwnerActor->GetInstigator())
	{
		if (Target == Inst)
		{
			return false;
		}
	}

	// 중복 히트 방지
	if (HitActors.Contains(Target))
	{
		return false;
	}

	return true;
}

void UMSProjectileBehavior_Normal::HandleHitTarget(
	AActor* Target,
	const FHitResult& HitResult
)
{
	// 히트 기록
	HitActors.Add(Target);

	// 여기에 나중에 추가:
	// - GameplayEffect 적용
	// - 넉백 / 경직
	// - HitResult 기반 방향 처리
	(void)HitResult;
}
