// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Projectile/Behaviors/MSPB_IceSpear.h"

#include "AbilitySystemComponent.h"
#include "AbilitysystemGlobals.h"
#include "Engine/EngineTypes.h"
#include "Engine/OverlapResult.h"
#include "GameplayEffect.h"
#include "MSGameplayTags.h"
#include "Actors/Projectile/MSBaseProjectile.h"
#include "Actors/Projectile/Behaviors/MSPB_Normal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "TimerManager.h"

void UMSPB_IceSpear::OnBegin_Implementation()
{
	HitActors.Reset();
	bHasSplit = false;

	AMSBaseProjectile* OwnerActor = GetOwnerActor();
	if (!OwnerActor)
	{
		return;
	}

	OwnerActor->PlaySFXAtLocation(0);

	UProjectileMovementComponent* MoveComp = OwnerActor->GetMovementComponent();
	if (!MoveComp)
	{
		return;
	}

	MoveComp->bInitialVelocityInLocalSpace = false;
	MoveComp->InitialSpeed = RuntimeData.ProjectileSpeed;
	MoveComp->MaxSpeed = RuntimeData.ProjectileSpeed;
	MoveComp->bRotationFollowsVelocity = true;
	MoveComp->bShouldBounce = false;
	MoveComp->Bounciness = 0.f;
	MoveComp->ProjectileGravityScale = 0.f;

	FVector Dir = RuntimeData.Direction.GetSafeNormal();
	if (Dir.IsNearlyZero())
	{
		Dir = OwnerActor->GetActorForwardVector();
	}

	MoveComp->Velocity = RuntimeData.ProjectileSpeed * Dir;

	RemainingPenetration = RuntimeData.PenetrationCount;
}

void UMSPB_IceSpear::OnTargetEnter_Implementation(
	AActor* Target,
	const FHitResult& HitResult
)
{
	if (!IsAuthority())
	{
		return;
	}

	if (!CanHitTarget(Target))
	{
		return;
	}

	HandleHitTarget(Target, HitResult);

	float FinalDamage = RuntimeData.Damage;
	const bool bIsCritical = FMath::FRand() < RuntimeData.CriticalChance;
	if (bIsCritical)
	{
		FinalDamage *= RuntimeData.CriticalDamage;
	}

	UAbilitySystemComponent* TargetASC =
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target);
	if (!TargetASC || !RuntimeData.DamageEffect)
	{
		return;
	}

	FGameplayEffectContextHandle Context = TargetASC->MakeEffectContext();

	if (AMSBaseProjectile* OwnerProj = GetOwnerActor())
	{
		Context.AddSourceObject(OwnerProj);
	}
	Context.AddHitResult(HitResult);

	FGameplayEffectSpecHandle SpecHandle =
		TargetASC->MakeOutgoingSpec(RuntimeData.DamageEffect, 1.f, Context);
	if (!SpecHandle.IsValid())
	{
		return;
	}

	SpecHandle.Data->SetSetByCallerMagnitude(
		MSGameplayTags::Data_Damage,
		(FinalDamage * -1.f)
	);

	if (bIsCritical)
	{
		SpecHandle.Data->AddDynamicAssetTag(MSGameplayTags::Hit_Critical);
	}

	TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);

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

	const bool bDidSplit = TrySplitOnFirstHit(Target, HitResult);
	if (bDidSplit)
	{
		if (AMSBaseProjectile* OwnerActor = GetOwnerActor())
		{
			OwnerActor->EnableCollision(false);
			OwnerActor->SetActorHiddenInGame(true);
			OwnerActor->SetLifeSpan(0.2f);
		}
		return;
	}

	AMSBaseProjectile* OwnerActor = GetOwnerActor();
	if (!OwnerActor)
	{
		return;
	}

	if (RemainingPenetration <= 0)
	{
		OwnerActor->RequestDestroy();
		return;
	}

	RemainingPenetration--;
}

void UMSPB_IceSpear::OnEnd_Implementation()
{
	HitActors.Reset();
}

bool UMSPB_IceSpear::CanHitTarget(AActor* Target) const
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

	if (Target == OwnerActor)
	{
		return false;
	}

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

	if (HitActors.Contains(Target))
	{
		return false;
	}

	return true;
}

void UMSPB_IceSpear::HandleHitTarget(
	AActor* Target,
	const FHitResult& HitResult
)
{
	HitActors.Add(Target);
	(void)HitResult;
}

bool UMSPB_IceSpear::TrySplitOnFirstHit(AActor* HitTarget, const FHitResult& HitResult)
{
	if (bHasSplit)
	{
		return false;
	}

	bHasSplit = true;

	AMSBaseProjectile* OwnerActor = GetOwnerActor();
	if (!OwnerActor)
	{
		return false;
	}

	UWorld* World = OwnerActor->GetWorld();
	if (!World)
	{
		return false;
	}

	FVector Origin;
	if (HitResult.ImpactPoint.IsNearlyZero())
	{
		Origin = OwnerActor->GetActorLocation();
	}
	else
	{
		Origin = FVector(HitResult.ImpactPoint);
	}

	FVector BaseDir = RuntimeData.Direction.GetSafeNormal();
	if (BaseDir.IsNearlyZero())
	{
		BaseDir = OwnerActor->GetActorForwardVector();
	}
	BaseDir.Z = 0.f;
	BaseDir = BaseDir.GetSafeNormal();
	if (BaseDir.IsNearlyZero())
	{
		BaseDir = FVector::ForwardVector;
	}

	constexpr float SplitRadius = 700.f;

	FCollisionObjectQueryParams ObjParams;
	ObjParams.AddObjectTypesToQuery(ECC_GameTraceChannel3);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(IceSpearSplit), false);
	QueryParams.AddIgnoredActor(OwnerActor);
	QueryParams.AddIgnoredActor(HitTarget);
	if (AActor* OwnerActorOwner = OwnerActor->GetOwner())
	{
		QueryParams.AddIgnoredActor(OwnerActorOwner);
	}
	if (APawn* Inst = OwnerActor->GetInstigator())
	{
		QueryParams.AddIgnoredActor(Inst);
	}

	TArray<FOverlapResult> Hits;
	const bool bAnyHit = World->OverlapMultiByObjectType(
		Hits,
		Origin,
		FQuat::Identity,
		ObjParams,
		FCollisionShape::MakeSphere(SplitRadius),
		QueryParams
	);

	AActor* BestTarget = nullptr;
	AActor* SecondTarget = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();
	float SecondDistSq = TNumericLimits<float>::Max();

	if (bAnyHit)
	{
		for (const FOverlapResult& H : Hits)
		{
			AActor* Candidate = H.GetActor();
			if (!Candidate)
			{
				continue;
			}

			const float DistSq = FVector::DistSquared(Origin, Candidate->GetActorLocation());
			if (DistSq < BestDistSq)
			{
				SecondTarget = BestTarget;
				SecondDistSq = BestDistSq;
				BestTarget = Candidate;
				BestDistSq = DistSq;
			}
			else if (DistSq < SecondDistSq)
			{
				SecondTarget = Candidate;
				SecondDistSq = DistSq;
			}
		}
	}

	const int32 NextPenetration = FMath::Max(RemainingPenetration, 0);

	bool bSpawned = false;
	if (BestTarget && SecondTarget)
	{
		FVector DirA = (BestTarget->GetActorLocation() - Origin);
		DirA.Z = 0.f;
		DirA = DirA.GetSafeNormal();

		FVector DirB = (SecondTarget->GetActorLocation() - Origin);
		DirB.Z = 0.f;
		DirB = DirB.GetSafeNormal();

		OwnerActor->TriggerSplitEvent(Origin, DirA, DirB, NextPenetration);

		bSpawned |= SpawnSplitProjectileDir(DirA, Origin, NextPenetration, HitTarget, false);
		bSpawned |= SpawnSplitProjectileDir(DirB, Origin, NextPenetration, HitTarget, false);
	}
	else if (BestTarget)
	{
		FVector DirA = (BestTarget->GetActorLocation() - Origin);
		DirA.Z = 0.f;
		DirA = DirA.GetSafeNormal();

		const float Dot = FVector::DotProduct(DirA, BaseDir);
		const FVector Proj = BaseDir * Dot;
		FVector DirB = (2.0f * Proj) - DirA;
		DirB.Z = 0.f;
		DirB = DirB.GetSafeNormal();
		if (DirB.IsNearlyZero())
		{
			DirB = -DirA;
		}

		OwnerActor->TriggerSplitEvent(Origin, DirA, DirB, NextPenetration);

		bSpawned |= SpawnSplitProjectileDir(DirA, Origin, NextPenetration, HitTarget, false);
		bSpawned |= SpawnSplitProjectileDir(DirB, Origin, NextPenetration, HitTarget, false);
	}
	else
	{
		const FVector DirA = BaseDir.RotateAngleAxis(90.f, FVector::UpVector);
		const FVector DirB = BaseDir.RotateAngleAxis(-90.f, FVector::UpVector);
		OwnerActor->TriggerSplitEvent(Origin, DirA, DirB, NextPenetration);

		bSpawned |= SpawnSplitProjectileDir(DirA, Origin, NextPenetration, HitTarget, false);
		bSpawned |= SpawnSplitProjectileDir(DirB, Origin, NextPenetration, HitTarget, false);
	}

	return bSpawned;
}

bool UMSPB_IceSpear::SpawnSplitProjectileDir(
	const FVector& Dir,
	const FVector& Origin,
	int32 NextPenetration,
	AActor* IgnoreActor,
	bool bReplicate
)
{
	AMSBaseProjectile* OwnerActor = GetOwnerActor();
	if (!OwnerActor || !OwnerActor->ProjectileDataClass)
	{
		return false;
	}

	UWorld* World = OwnerActor->GetWorld();
	if (!World)
	{
		return false;
	}

	FVector LaunchDir = Dir;
	LaunchDir.Z = 0.f;
	LaunchDir = LaunchDir.GetSafeNormal();
	if (LaunchDir.IsNearlyZero())
	{
		return false;
	}

	FProjectileRuntimeData SplitData = RuntimeData;
	SplitData.Direction = LaunchDir;
	SplitData.PenetrationCount = NextPenetration;
	SplitData.BehaviorClass = UMSPB_Normal::StaticClass();

	const float SpawnOffset = 50.f;
	const FVector SpawnLocation = Origin + (LaunchDir * SpawnOffset);
	const FTransform SpawnTransform(LaunchDir.Rotation(), SpawnLocation);

	AMSBaseProjectile* SplitProjectile = World->SpawnActorDeferred<AMSBaseProjectile>(
		AMSBaseProjectile::StaticClass(),
		SpawnTransform,
		OwnerActor->GetOwner(),
		OwnerActor->GetInstigator(),
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
	);

	if (!SplitProjectile)
	{
		return false;
	}

	if (!bReplicate)
	{
		SplitProjectile->SetReplicates(false);
		SplitProjectile->SetReplicateMovement(false);
	}

	static const FName IceSpearSplitTag(TEXT("IceSpearSplit"));
	SplitProjectile->Tags.AddUnique(IceSpearSplitTag);
	SplitProjectile->ProjectileDataClass = OwnerActor->ProjectileDataClass;
	SplitProjectile->SetProjectileRuntimeData(SplitData);
	SplitProjectile->AddIgnoredActor(IgnoreActor);
	SplitProjectile->EnableCollision(false);
	SplitProjectile->FinishSpawning(SpawnTransform);

	TWeakObjectPtr<AMSBaseProjectile> WeakProjectile = SplitProjectile;
	World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([WeakProjectile]()
		{
			if (WeakProjectile.IsValid())
			{
				WeakProjectile->EnableCollision(true);
			}
		}));

	return true;
}
