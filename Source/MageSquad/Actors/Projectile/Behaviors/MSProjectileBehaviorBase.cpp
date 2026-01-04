// Fill out your copyright notice in the Description page of Project Settings.

#include "MSProjectileBehaviorBase.h"
#include "Actors/Projectile/MSBaseProjectile.h"
#include "Engine/World.h"

void UMSProjectileBehaviorBase::Initialize(AMSBaseProjectile* InOwner, const FProjectileRuntimeData& InRuntimeData)
{
	Owner = InOwner;
	// Owner에서 유효한(RuntimeData + StaticData fallback) 값을 받아 캐시
	if (AMSBaseProjectile* OwnerActor = Owner.Get())
	{
		RuntimeData = OwnerActor->GetEffectiveRuntimeData();
	}
	else
	{
		// 혹시라도 Owner가 없으면 전달받은 값 사용
		RuntimeData = InRuntimeData;
	}
}

bool UMSProjectileBehaviorBase::IsAuthority() const
{
	if (const AMSBaseProjectile* OwnerActor = Owner.Get())
	{
		return OwnerActor->HasAuthority();
	}
	return false;
}

UWorld* UMSProjectileBehaviorBase::GetWorldSafe() const
{
	if (const AMSBaseProjectile* OwnerActor = Owner.Get())
	{
		return OwnerActor->GetWorld();
	}
	return nullptr;
}

void UMSProjectileBehaviorBase::ApplyCollisionRadius(AMSBaseProjectile* InOwner, const FProjectileRuntimeData& InRuntimeData)
{
	if (!InOwner)
	{
		return;
	}

	const float CollisionRadius = (InRuntimeData.Radius > 0.f) ? InRuntimeData.Radius : 100.f;
	InOwner->SetCollisionRadius(CollisionRadius);
}
