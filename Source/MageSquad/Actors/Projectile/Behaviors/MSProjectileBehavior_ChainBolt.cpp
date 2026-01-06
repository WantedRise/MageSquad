// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Projectile/Behaviors/MSProjectileBehavior_ChainBolt.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "MSGameplayTags.h"
#include "Actors/Projectile/MSBaseProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMSProjectileBehavior_ChainBolt::OnBegin_Implementation()
{
	AMSBaseProjectile* OwnerActor = GetOwnerActor();
	if (!OwnerActor)
	{
		return;
	}

	OwnerActor->EnableCollision(false);

	if (!IsAuthority())
	{
		return;
	}

	AActor* SourceActor = OwnerActor->GetOwner();
	if (!SourceActor)
	{
		SourceActor = OwnerActor->GetInstigator();
	}
	if (!SourceActor)
	{
		EndChain();
		return;
	}

	ChainSourceActor = SourceActor;
	HitActors.Reset();
	LastHitActor = nullptr;
	RemainingChains = MaxChains;
	OwnerActor->SetActorLocation(SourceActor->GetActorLocation());

	PerformChainStep();
}

void UMSProjectileBehavior_ChainBolt::OnTargetEnter_Implementation(AActor* Target, const FHitResult& HitResult)
{
	(void)Target;
	(void)HitResult;
}

void UMSProjectileBehavior_ChainBolt::OnEnd_Implementation()
{
	if (UWorld* World = GetWorldSafe())
	{
		World->GetTimerManager().ClearTimer(ChainTimerHandle);
		World->GetTimerManager().ClearTimer(TravelTimerHandle);
	}
	HitActors.Reset();
}

void UMSProjectileBehavior_ChainBolt::ApplyCollisionRadius(
	AMSBaseProjectile* InOwner, const FProjectileRuntimeData& InRuntimeData)
{
	if (InOwner)
	{
		InOwner->SetCollisionRadius(1.f);
	}
}

void UMSProjectileBehavior_ChainBolt::PerformChainStep()
{
	if (!IsAuthority())
	{
		EndChain();
		return;
	}

	if (RemainingChains <= 0)
	{
		EndChain();
		return;
	}

	AActor* OriginActor = LastHitActor.IsValid() ? LastHitActor.Get() : ChainSourceActor.Get();
	if (!OriginActor)
	{
		EndChain();
		return;
	}

	const float DefaultRange = (RuntimeData.Radius > 0.f) ? RuntimeData.Radius : 800.f;
	const float SearchRange = LastHitActor.IsValid() ? DefaultRange : InitialChainRange;
	AActor* Target = FindClosestTarget(OriginActor->GetActorLocation(), SearchRange);
	if (!Target)
	{
		EndChain();
		return;
	}

	AMSBaseProjectile* OwnerActor = GetOwnerActor();
	if (!OwnerActor)
	{
		EndChain();
		return;
	}

	PendingTarget = Target;

	const FVector StartLocation = OriginActor->GetActorLocation();
	const FVector TargetLocation = Target->GetActorLocation();
	const FVector Direction = (TargetLocation - StartLocation).GetSafeNormal();
	const float Speed = FMath::Max(RuntimeData.ProjectileSpeed, 1.f);
	const float Distance = FVector::Dist(StartLocation, TargetLocation);
	const float TravelTime = (Distance > 0.f) ? (Distance / Speed) : 0.f;

	OwnerActor->SetActorLocation(StartLocation);
	if (UProjectileMovementComponent* MoveComp = OwnerActor->GetMovementComponent())
	{
		MoveComp->Velocity = Direction * Speed;
	}

	if (TravelTime <= KINDA_SMALL_NUMBER)
	{
		HandleArrival();
		return;
	}

	if (UWorld* World = GetWorldSafe())
	{
		World->GetTimerManager().SetTimer(
			TravelTimerHandle,
			this,
			&UMSProjectileBehavior_ChainBolt::HandleArrival,
			TravelTime,
			false
		);
	}
}

AActor* UMSProjectileBehavior_ChainBolt::FindClosestTarget(
	const FVector& Origin,
	float SearchRadius) const
{
	UWorld* World = GetWorldSafe();
	if (!World)
	{
		return nullptr;
	}

	FCollisionObjectQueryParams ObjParams;
	ObjParams.AddObjectTypesToQuery(ECC_GameTraceChannel3); // MSEnemy

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ChainBoltOverlap), false);
	if (const AMSBaseProjectile* OwnerProj = GetOwnerActor())
	{
		QueryParams.AddIgnoredActor(OwnerProj);
	}

	TArray<FOverlapResult> Hits;
	const bool bAnyHit = World->OverlapMultiByObjectType(
		Hits,
		Origin,
		FQuat::Identity,
		ObjParams,
		FCollisionShape::MakeSphere(SearchRadius),
		QueryParams
	);

	if (!bAnyHit)
	{
		return nullptr;
	}

	AActor* BestTarget = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();

	for (const FOverlapResult& H : Hits)
	{
		AActor* Candidate = H.GetActor();
		if (!Candidate || HitActors.Contains(Candidate))
		{
			continue;
		}

		const float DistSq = FVector::DistSquared(Origin, Candidate->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			BestTarget = Candidate;
		}
	}

	return BestTarget;
}

void UMSProjectileBehavior_ChainBolt::HandleArrival()
{
	if (!IsAuthority())
	{
		EndChain();
		return;
	}

	AActor* Target = PendingTarget.Get();
	if (!Target)
	{
		EndChain();
		return;
	}

	AMSBaseProjectile* OwnerActor = GetOwnerActor();
	if (!OwnerActor)
	{
		EndChain();
		return;
	}

	OwnerActor->StopMovement();
	OwnerActor->SetActorLocation(Target->GetActorLocation());

	HitActors.Add(Target);
	LastHitActor = Target;
	RemainingChains--;
	PendingTarget = nullptr;

	ApplyDamageToTarget(Target, RuntimeData.Damage);

	if (RuntimeData.OnHitVFX)
	{
		const FVector Location = Target->GetActorLocation();
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorldSafe(),
			RuntimeData.OnHitVFX,
			Location
		);
	}

	if (RemainingChains > 0)
	{
		if (UWorld* World = GetWorldSafe())
		{
			World->GetTimerManager().SetTimer(
				ChainTimerHandle,
				this,
				&UMSProjectileBehavior_ChainBolt::PerformChainStep,
				ChainInterval,
				false
			);
		}
	}
	else
	{
		EndChain();
	}
}

void UMSProjectileBehavior_ChainBolt::ApplyDamageToTarget(AActor* Target, float DamageAmount)
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

	SpecHandle.Data->SetSetByCallerMagnitude(MSGameplayTags::Data_Damage, (DamageAmount * -1.f));
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

void UMSProjectileBehavior_ChainBolt::EndChain()
{
	if (AMSBaseProjectile* OwnerActor = GetOwnerActor())
	{
		OwnerActor->Destroy();
	}
}
