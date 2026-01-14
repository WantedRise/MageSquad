// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Projectile/Behaviors/MSPB_TrailDoT.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "MSGameplayTags.h"
#include "Actors/Projectile/MSBaseProjectile.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMSPB_TrailDoT::OnBegin_Implementation()
{
	DirectHitActors.Reset();
	TrailPoints.Reset();

	AMSBaseProjectile* OwnerActor = GetOwnerActor();
	if (!OwnerActor)
	{
		return;
	}

	OwnerActor->RequestSpawnSFX();

	StartLocation = OwnerActor->GetActorLocation();
	VirtualLocation = StartLocation;
	TrailPoints.Add(StartLocation);
	bMovementStopped = false;

	UProjectileMovementComponent* MoveComp = OwnerActor->GetMovementComponent();
	if (!MoveComp)
	{
		return;
	}

	MoveComp->StopMovementImmediately();
	MoveComp->Velocity = FVector::ZeroVector;

	TravelDirection = RuntimeData.Direction.GetSafeNormal();
	if (TravelDirection.IsNearlyZero())
	{
		TravelDirection = OwnerActor->GetActorForwardVector();
	}

	OwnerActor->EnableCollision(false);

	if (!IsAuthority())
	{
		return;
	}

	if (UWorld* World = OwnerActor->GetWorld())
	{
		World->GetTimerManager().SetTimer(
			TrailSampleTimerHandle,
			this,
			&UMSPB_TrailDoT::SampleTrailPoint,
			TrailSampleInterval,
			true
		);

		const float DamageInterval = (RuntimeData.DamageInterval > 0.f) ? RuntimeData.DamageInterval : 0.5f;
		World->GetTimerManager().SetTimer(
			TrailDamageTimerHandle,
			this,
			&UMSPB_TrailDoT::TickTrailDamage,
			DamageInterval,
			true
		);

		const float TrailLifetime = (RuntimeData.LifeTime > 0.f) ? RuntimeData.LifeTime : DefaultTrailLifetime;
		World->GetTimerManager().SetTimer(
			TrailEndTimerHandle,
			this,
			&UMSPB_TrailDoT::EndTrail,
			TrailLifetime,
			false
		);
	}
}

void UMSPB_TrailDoT::OnTargetEnter_Implementation(AActor* Target, const FHitResult& HitResult)
{
	if (!IsAuthority())
	{
		return;
	}

	if (!CanHitDirectTarget(Target))
	{
		return;
	}

	DirectHitActors.Add(Target);
	ApplyDamageToTarget(Target, RuntimeData.Damage);
	(void)HitResult;
}

void UMSPB_TrailDoT::OnEnd_Implementation()
{
	if (AMSBaseProjectile* OwnerActor = GetOwnerActor())
	{
		if (UWorld* World = OwnerActor->GetWorld())
		{
			World->GetTimerManager().ClearTimer(TrailSampleTimerHandle);
			World->GetTimerManager().ClearTimer(TrailDamageTimerHandle);
			World->GetTimerManager().ClearTimer(TrailEndTimerHandle);
		}
	}

	TrailPoints.Reset();
	DirectHitActors.Reset();
}

void UMSPB_TrailDoT::ApplyCollisionRadius(AMSBaseProjectile* InOwner, const FProjectileRuntimeData& InRuntimeData)
{
	if (!InOwner)
	{
		return;
	}

	const float Base = (InRuntimeData.Radius > 0.f) ? InRuntimeData.Radius : 100.f;
	InOwner->SetCollisionRadius(Base * 0.15f);
}

void UMSPB_TrailDoT::SampleTrailPoint()
{
	if (!IsAuthority())
	{
		return;
	}

	AMSBaseProjectile* OwnerActor = GetOwnerActor();
	if (!OwnerActor)
	{
		return;
	}

	if (bMovementStopped)
	{
		return;
	}

	const float Step = RuntimeData.ProjectileSpeed * TrailSampleInterval;
	VirtualLocation += TravelDirection * Step;
	TrailPoints.Add(VirtualLocation);
	HandleDirectHitAtPoint(VirtualLocation);

	const float MaxDistance = RuntimeData.Radius;
	if (MaxDistance > 0.f)
	{
		const float Dist = FVector::Dist(StartLocation, VirtualLocation);
		if (Dist >= MaxDistance)
		{
			bMovementStopped = true;
			if (UWorld* World = OwnerActor->GetWorld())
			{
				World->GetTimerManager().ClearTimer(TrailSampleTimerHandle);
			}
		}
	}
}

void UMSPB_TrailDoT::TickTrailDamage()
{
	if (!IsAuthority())
	{
		return;
	}

	AMSBaseProjectile* OwnerActor = GetOwnerActor();
	if (!OwnerActor)
	{
		return;
	}

	if (!RuntimeData.DamageEffect || TrailPoints.Num() == 0)
	{
		return;
	}

	const float TrailRadius = ((RuntimeData.Radius > 0.f) ? RuntimeData.Radius : 100.f) * 0.15f;
	const float DotDamage = RuntimeData.Damage * 0.2f;

	UWorld* World = OwnerActor->GetWorld();
	if (!World || TrailRadius <= 0.f)
	{
		return;
	}

	TSet<AActor*> UniqueTargets;
	for (const FVector& Point : TrailPoints)
	{
		TArray<FOverlapResult> Hits;
		FCollisionObjectQueryParams ObjParams;
		ObjParams.AddObjectTypesToQuery(ECC_GameTraceChannel3); // MSEnemy

		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(TrailDotOverlap), false);
		QueryParams.AddIgnoredActor(OwnerActor);

		const bool bAnyHit = World->OverlapMultiByObjectType(
			Hits,
			Point,
			FQuat::Identity,
			ObjParams,
			FCollisionShape::MakeSphere(TrailRadius),
			QueryParams
		);

		if (!bAnyHit)
		{
			continue;
		}

		for (const FOverlapResult& H : Hits)
		{
			if (AActor* HitActor = H.GetActor())
			{
				UniqueTargets.Add(HitActor);
			}
		}
	}

	for (AActor* Target : UniqueTargets)
	{
		ApplyDamageToTarget(Target, DotDamage);
	}
}

void UMSPB_TrailDoT::EndTrail()
{
	if (AMSBaseProjectile* OwnerActor = GetOwnerActor())
	{
		OwnerActor->RequestDestroy();
	}
}

void UMSPB_TrailDoT::HandleDirectHitAtPoint(const FVector& Point)
{
	if (!IsAuthority() || RuntimeData.DamageEffect == nullptr)
	{
		return;
	}

	AMSBaseProjectile* OwnerActor = GetOwnerActor();
	if (!OwnerActor)
	{
		return;
	}

	const float HitRadius = ((RuntimeData.Radius > 0.f) ? RuntimeData.Radius : 100.f) * 0.15f;
	if (HitRadius <= 0.f)
	{
		return;
	}

	UWorld* World = OwnerActor->GetWorld();
	if (!World)
	{
		return;
	}

	TArray<FOverlapResult> Hits;
	FCollisionObjectQueryParams ObjParams;
	ObjParams.AddObjectTypesToQuery(ECC_GameTraceChannel3); // MSEnemy

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(TrailDirectOverlap), false);
	QueryParams.AddIgnoredActor(OwnerActor);

	const bool bAnyHit = World->OverlapMultiByObjectType(
		Hits,
		Point,
		FQuat::Identity,
		ObjParams,
		FCollisionShape::MakeSphere(HitRadius),
		QueryParams
	);

	if (!bAnyHit)
	{
		return;
	}

	for (const FOverlapResult& H : Hits)
	{
		AActor* HitActor = H.GetActor();
		if (!HitActor || DirectHitActors.Contains(HitActor))
		{
			continue;
		}

		if (!CanHitDirectTarget(HitActor))
		{
			continue;
		}

		DirectHitActors.Add(HitActor);
		ApplyDamageToTarget(HitActor, RuntimeData.Damage);
	}
}

bool UMSPB_TrailDoT::CanHitDirectTarget(AActor* Target) const
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

	if (Target == OwnerActor || Target == OwnerActor->GetOwner())
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

	if (DirectHitActors.Contains(Target))
	{
		return false;
	}

	return true;
}

void UMSPB_TrailDoT::ApplyDamageToTarget(AActor* Target, float DamageAmount)
{
	if (!Target || !RuntimeData.DamageEffect)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC =
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target);

	if (!TargetASC)
	{
		return;
	}

	FGameplayEffectContextHandle Context = TargetASC->MakeEffectContext();
	if (AMSBaseProjectile* OwnerProj = GetOwnerActor())
	{
		Context.AddSourceObject(OwnerProj);
	}

	FGameplayEffectSpecHandle SpecHandle =
		TargetASC->MakeOutgoingSpec(RuntimeData.DamageEffect, 1.f, Context);
	if (!SpecHandle.IsValid())
	{
		return;
	}

	float FinalDamage = DamageAmount;
	const bool bIsCritical = FMath::FRand() < RuntimeData.CriticalChance;
	if (bIsCritical)
	{
		FinalDamage *= RuntimeData.CriticalDamage;
	}

	SpecHandle.Data->SetSetByCallerMagnitude(MSGameplayTags::Data_Damage, (FinalDamage * -1.f));
	if (bIsCritical)
	{
		SpecHandle.Data->AddDynamicAssetTag(MSGameplayTags::Hit_Critical);
	}
	TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

	for (const TSubclassOf<UGameplayEffect>& ExtraEffect : RuntimeData.Effects)
	{
		if (!ExtraEffect)
		{
			continue;
		}

		FGameplayEffectSpecHandle ExtraSpec =
			TargetASC->MakeOutgoingSpec(ExtraEffect, 1.f, Context);
		if (!ExtraSpec.IsValid())
		{
			continue;
		}

		TargetASC->ApplyGameplayEffectSpecToSelf(*ExtraSpec.Data.Get());
	}
}

