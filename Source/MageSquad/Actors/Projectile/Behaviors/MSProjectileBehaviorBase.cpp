// Fill out your copyright notice in the Description page of Project Settings.

#include "MSProjectileBehaviorBase.h"
#include "Actors/Projectile/MSBaseProjectile.h"
#include "Engine/World.h"

void UMSProjectileBehaviorBase::Initialize(AMSBaseProjectile* InOwner, const FProjectileRuntimeData& InRuntimeData)
{
	Owner = InOwner;
	RuntimeData = InRuntimeData;
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