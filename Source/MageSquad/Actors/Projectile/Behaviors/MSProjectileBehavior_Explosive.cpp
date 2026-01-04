// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Projectile/Behaviors/MSProjectileBehavior_Explosive.h"
#include "Components/PrimitiveComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/Globals/MSAbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "MSGameplayTags.h"
#include "NiagaraFunctionLibrary.h"
#include "Actors/Projectile/MSBaseProjectile.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/ProjectileMovementComponent.h"

void UMSProjectileBehavior_Explosive::OnBegin_Implementation()
{
	AMSBaseProjectile* OwnerActor = GetOwnerActor();
	if (!OwnerActor)
	{
		return;
	}

	UProjectileMovementComponent* MoveComp = OwnerActor->GetMovementComponent();
	if (!MoveComp)
	{
		return;
	}

	if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(OwnerActor->GetRootComponent()))
	{
		RootPrim->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Block);
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
}

void UMSProjectileBehavior_Explosive::OnTargetEnter_Implementation(AActor* Target, const FHitResult& HitResult)
{
	if (bExploded)
	{
		return;
	}

	AMSBaseProjectile* OwnerActor = GetOwnerActor();
	if (!OwnerActor)
	{
		return;
	}

	bExploded = true;

	FVector ExplosionOrigin = OwnerActor->GetActorLocation();
	if (!HitResult.ImpactPoint.IsNearlyZero())
	{
		ExplosionOrigin = HitResult.ImpactPoint;
	}
	else if (Target)
	{
		ExplosionOrigin = Target->GetActorLocation();
	}

	OwnerActor->SetActorLocation(ExplosionOrigin);

	if (!IsAuthority())
	{
		OwnerActor->StopMovement();
		OwnerActor->EnableCollision(false);
		OwnerActor->SetActorHiddenInGame(true);
		return;
	}

	OwnerActor->Multicast_StopAndHide(ExplosionOrigin);
	if (RuntimeData.OnHitVFX)
	{
		const float VfxScale = FMath::Max(0.01f, RuntimeData.Radius / 200.f);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			OwnerActor,
			RuntimeData.OnHitVFX,
			ExplosionOrigin,
			FRotator::ZeroRotator,
			FVector(VfxScale)
		);
	}


	const float Radius = RuntimeData.Radius;
	UWorld* World = GetWorldSafe();
	if (World && Radius > 0.f)
	{
		FCollisionObjectQueryParams ObjParams;
		ObjParams.AddObjectTypesToQuery(ECC_GameTraceChannel3); // MSEnemy

		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ExplosiveOverlap), false);
		QueryParams.AddIgnoredActor(OwnerActor);

		TArray<FOverlapResult> Hits;
		const bool bAnyHit = World->OverlapMultiByObjectType(
			Hits,
			ExplosionOrigin,
			FQuat::Identity,
			ObjParams,
			FCollisionShape::MakeSphere(Radius),
			QueryParams
		);

		if (bAnyHit)
		{
			for (const FOverlapResult& H : Hits)
			{
				AActor* HitActor = H.GetActor();
				if (!HitActor)
				{
					continue;
				}

				if (HitActors.Contains(HitActor))
				{
					continue;
				}
				HitActors.Add(HitActor);

				ApplyDamageToTarget(HitActor, RuntimeData.Damage);
			}
		}
	}

	OwnerActor->Destroy();
}

void UMSProjectileBehavior_Explosive::OnEnd_Implementation()
{
	HitActors.Reset();
}

void UMSProjectileBehavior_Explosive::ApplyCollisionRadius(AMSBaseProjectile* InOwner, const FProjectileRuntimeData& InRuntimeData)
{
	if (!InOwner)
	{
		return;
	}

	InOwner->SetCollisionRadius(50.f);
}

void UMSProjectileBehavior_Explosive::ApplyDamageToTarget(AActor* Target, float DamageAmount)
{
	if (!Target || !RuntimeData.DamageEffect)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = nullptr;
	if (Target->GetClass()->ImplementsInterface(UAbilitySystemInterface::StaticClass()))
	{
		TargetASC = Cast<IAbilitySystemInterface>(Target)->GetAbilitySystemComponent();
	}
	if (!TargetASC)
	{
		TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target);
	}
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
}




