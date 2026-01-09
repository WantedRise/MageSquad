// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Projectile/Behaviors/MSPB_OrbitBlade.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "MSGameplayTags.h"
#include "Actors/Projectile/MSBaseProjectile.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMSPB_OrbitBlade::OnBegin_Implementation()
{
	AMSBaseProjectile* OwnerActor = GetOwnerActor();
	if (!OwnerActor)
	{
		return;
	}

	AActor* CenterActor = OwnerActor->GetOwner();
	if (!CenterActor)
	{
		CenterActor = OwnerActor->GetInstigator();
	}
	if (!CenterActor)
	{
		return;
	}

	OrbitCenter = CenterActor;

	FVector Dir = RuntimeData.Direction.GetSafeNormal();
	if (Dir.IsNearlyZero())
	{
		Dir = OwnerActor->GetActorForwardVector();
	}

	CurrentAngleRad = FMath::Atan2(Dir.Y, Dir.X);
	AngularSpeedRad = FMath::DegreesToRadians(RuntimeData.ProjectileSpeed > 0.f ? RuntimeData.ProjectileSpeed : 180.f);
	OrbitRadius = 700.f;
	if (RuntimeData.OptionalParameters.Num() > 0)
	{
		const float SignedRadius = RuntimeData.OptionalParameters[0];
		if (!FMath::IsNearlyZero(SignedRadius))
		{
			const float SpeedSign = (SignedRadius < 0.f) ? -1.f : 1.f;
			AngularSpeedRad *= SpeedSign;
			OrbitRadius = FMath::Abs(SignedRadius);
		}
	}

	OwnerActor->EnableCollision(true);

	const FVector CenterLoc = CenterActor->GetActorLocation();
	const FVector Offset(FMath::Cos(CurrentAngleRad), FMath::Sin(CurrentAngleRad), 0.f);
	OwnerActor->SetActorLocation(CenterLoc + Offset * OrbitRadius, true);

	const float VfxScale = (RuntimeData.Radius > 0.f) ? (RuntimeData.Radius / 70.f) : 1.f;
	ApplyVfxScale(VfxScale);

	if (UWorld* World = OwnerActor->GetWorld())
	{
		World->GetTimerManager().SetTimer(
			OrbitTimerHandle,
			this,
			&UMSPB_OrbitBlade::TickOrbit,
			0.016f,
			true
		);

		const float LifeTime = (RuntimeData.LifeTime > 0.f) ? RuntimeData.LifeTime : 5.f;
		World->GetTimerManager().SetTimer(
			OrbitEndTimerHandle,
			this,
			&UMSPB_OrbitBlade::EndOrbit,
			LifeTime,
			false
		);
	}
}

void UMSPB_OrbitBlade::OnTargetEnter_Implementation(AActor* Target, const FHitResult& HitResult)
{
	if (!IsAuthority())
	{
		return;
	}

	if (!Target)
	{
		return;
	}

	ApplyDamageToTarget(Target, RuntimeData.Damage);
	(void)HitResult;
}

void UMSPB_OrbitBlade::OnEnd_Implementation()
{
	if (UWorld* World = GetWorldSafe())
	{
		World->GetTimerManager().ClearTimer(OrbitTimerHandle);
		World->GetTimerManager().ClearTimer(OrbitEndTimerHandle);
	}

}

void UMSPB_OrbitBlade::ApplyCollisionRadius(AMSBaseProjectile* InOwner, const FProjectileRuntimeData& InRuntimeData)
{
	if (!InOwner)
	{
		return;
	}

	const float Base = (InRuntimeData.Radius > 0.f) ? InRuntimeData.Radius : 150.f;
	InOwner->SetCollisionRadius(Base);
}

void UMSPB_OrbitBlade::TickOrbit()
{
	AMSBaseProjectile* OwnerActor = GetOwnerActor();
	AActor* CenterActor = OrbitCenter.Get();
	if (!OwnerActor || !CenterActor)
	{
		EndOrbit();
		return;
	}

	CurrentAngleRad += AngularSpeedRad * 0.016f;

	const FVector CenterLoc = CenterActor->GetActorLocation();
	const FVector Offset(FMath::Cos(CurrentAngleRad), FMath::Sin(CurrentAngleRad), 0.f);
	const FVector NewLoc = CenterLoc + Offset * OrbitRadius;
	OwnerActor->SetActorLocation(NewLoc, true);
}

void UMSPB_OrbitBlade::EndOrbit()
{
	if (AMSBaseProjectile* OwnerActor = GetOwnerActor())
	{
		OwnerActor->Destroy();
	}
}

void UMSPB_OrbitBlade::ApplyDamageToTarget(AActor* Target, float DamageAmount)
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

void UMSPB_OrbitBlade::ApplyVfxScale(float Scale)
{
	AMSBaseProjectile* OwnerActor = GetOwnerActor();
	if (!OwnerActor || Scale <= 0.f)
	{
		return;
	}

	TArray<UNiagaraComponent*> NiagaraComps;
	OwnerActor->GetComponents<UNiagaraComponent>(NiagaraComps);

	for (UNiagaraComponent* Comp : NiagaraComps)
	{
		if (Comp)
		{
			Comp->SetWorldScale3D(FVector(Scale));
		}
	}
}
