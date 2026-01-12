// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Projectile/Behaviors/MSPB_TornadoEnhanced.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Globals/MSAbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "MSFunctionLibrary.h"
#include "MSGameplayTags.h"
#include "Actors/Projectile/MSBaseProjectile.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

void UMSPB_TornadoEnhanced::OnBegin_Implementation()
{
	AMSBaseProjectile* OwnerProj = GetOwnerActor();
	if (!OwnerProj)
	{
		return;
	}

	bEnded = false;

	StartLocation = OwnerProj->GetActorLocation();
	ForwardDir = OwnerProj->GetActorForwardVector().GetSafeNormal();

	if (UWorld* World = OwnerProj->GetWorld())
	{
		StartTime = World->GetTimeSeconds();
	}
	if (RuntimeData.SFX.IsValidIndex(0) && RuntimeData.SFX[0])
	{
		LoopingSFX = UGameplayStatics::SpawnSoundAttached(RuntimeData.SFX[0], OwnerProj->GetRootComponent());
	}

	if (!OwnerProj->HasAuthority())
	{
		return;
	}

	StartMove();
	StartPeriodicDamage();
	StartSplit();
}

void UMSPB_TornadoEnhanced::OnEnd_Implementation()
{
	if (bEnded)
	{
		return;
	}
	bEnded = true;

	StopMove();
	StopPeriodicDamage();
	StopSplit();
	if (LoopingSFX.IsValid())
	{
		LoopingSFX->Stop();
		LoopingSFX = nullptr;
	}
}

void UMSPB_TornadoEnhanced::StartMove()
{
	AMSBaseProjectile* OwnerProj = GetOwnerActor();
	if (!OwnerProj)
	{
		return;
	}

	if (UWorld* World = OwnerProj->GetWorld())
	{
		World->GetTimerManager().ClearTimer(MoveTimerHandle);
		World->GetTimerManager().SetTimer(
			MoveTimerHandle,
			this,
			&UMSPB_TornadoEnhanced::TickMove,
			0.016f,
			true
		);
	}
}

void UMSPB_TornadoEnhanced::StopMove()
{
	if (UWorld* World = GetWorldSafe())
	{
		World->GetTimerManager().ClearTimer(MoveTimerHandle);
	}
}

void UMSPB_TornadoEnhanced::TickMove()
{
	AMSBaseProjectile* OwnerProj = GetOwnerActor();
	UWorld* World = GetWorldSafe();
	if (!OwnerProj || !World || !OwnerProj->HasAuthority())
	{
		return;
	}

	const float Now = World->GetTimeSeconds();
	const float T = Now - StartTime;

	const FVector Base = StartLocation + ForwardDir * (MoveSpeed * T);

	const FVector Right = OwnerProj->GetActorRightVector();
	const FVector Up = OwnerProj->GetActorUpVector();

	const float S1 = FMath::Sin(T * SwirlFreq);
	const float S2 = FMath::Cos(T * SwirlFreq * 0.9f);
	const float N  = FMath::PerlinNoise1D(T * NoiseFreq);

	const FVector Offset =
		Right * (S1 * SwirlAmp + N * NoiseAmp) +
		Up    * (S2 * (SwirlAmp * 0.25f));

	const FVector NewLoc = Base + Offset;

	OwnerProj->SetActorLocation(NewLoc, true);
}

void UMSPB_TornadoEnhanced::StartPeriodicDamage()
{
	AMSBaseProjectile* OwnerProj = GetOwnerActor();
	if (!OwnerProj)
	{
		return;
	}

	if (RuntimeData.DamageInterval <= 0.f)
	{
		return;
	}
	if (!RuntimeData.DamageEffect)
	{
		return;
	}

	TickPeriodicDamage();

	if (UWorld* World = OwnerProj->GetWorld())
	{
		World->GetTimerManager().ClearTimer(DamageTimerHandle);
		World->GetTimerManager().SetTimer(
			DamageTimerHandle,
			this,
			&UMSPB_TornadoEnhanced::TickPeriodicDamage,
			RuntimeData.DamageInterval,
			true
		);
	}
}

void UMSPB_TornadoEnhanced::StopPeriodicDamage()
{
	if (UWorld* World = GetWorldSafe())
	{
		World->GetTimerManager().ClearTimer(DamageTimerHandle);
	}
}

void UMSPB_TornadoEnhanced::TickPeriodicDamage()
{
	AMSBaseProjectile* OwnerProj = GetOwnerActor();
	if (!OwnerProj || !OwnerProj->HasAuthority())
	{
		OnEnd_Implementation();
		return;
	}

	const float DamageAmount = RuntimeData.Damage;

	UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(OwnerProj->GetRootComponent());
	if (!RootPrim)
	{
		return;
	}

	TArray<AActor*> OverlappingActors;
	RootPrim->GetOverlappingActors(OverlappingActors);

	TSet<AActor*> UniqueTargets;
	for (AActor* A : OverlappingActors)
	{
		if (!A || A == OwnerProj)
		{
			continue;
		}
		UniqueTargets.Add(A);
	}

	for (AActor* Target : UniqueTargets)
	{
		ApplyDamageToTarget(Target, DamageAmount);
	}
}

void UMSPB_TornadoEnhanced::ApplyDamageToTarget(AActor* Target, float DamageAmount)
{
	AMSBaseProjectile* OwnerProj = GetOwnerActor();
	if (!OwnerProj || !Target)
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
	Context.AddSourceObject(OwnerProj);

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

void UMSPB_TornadoEnhanced::StartSplit()
{
	AMSBaseProjectile* OwnerProj = GetOwnerActor();
	if (!OwnerProj)
	{
		return;
	}

	if (SplitInterval <= 0.f)
	{
		return;
	}

	if (UWorld* World = OwnerProj->GetWorld())
	{
		World->GetTimerManager().ClearTimer(SplitTimerHandle);
		World->GetTimerManager().SetTimer(
			SplitTimerHandle,
			this,
			&UMSPB_TornadoEnhanced::TickSplit,
			SplitInterval,
			true
		);
	}
}

void UMSPB_TornadoEnhanced::StopSplit()
{
	if (UWorld* World = GetWorldSafe())
	{
		World->GetTimerManager().ClearTimer(SplitTimerHandle);
	}
}

void UMSPB_TornadoEnhanced::TickSplit()
{
	AMSBaseProjectile* OwnerProj = GetOwnerActor();
	UWorld* World = GetWorldSafe();
	if (!OwnerProj || !World || !OwnerProj->HasAuthority())
	{
		return;
	}

	const float Now = World->GetTimeSeconds();
	float RemainingLife = RuntimeData.LifeTime;
	if (RemainingLife > 0.f)
	{
		const float Elapsed = Now - StartTime;
		RemainingLife = FMath::Max(0.f, RemainingLife - Elapsed);
		if (RemainingLife <= 0.f)
		{
			return;
		}
	}

	const FVector BaseDir = OwnerProj->GetActorForwardVector().GetSafeNormal();
	if (BaseDir.IsNearlyZero())
	{
		return;
	}

	const FVector DirA = BaseDir.RotateAngleAxis(SplitYawOffset, FVector::UpVector);
	const FVector DirB = BaseDir.RotateAngleAxis(-SplitYawOffset, FVector::UpVector);

	SpawnSplitProjectile(DirA, RemainingLife, SplitRadiusScale);
	SpawnSplitProjectile(DirB, RemainingLife, SplitRadiusScale);

	OnEnd_Implementation();
	OwnerProj->RequestDestroy();
}

void UMSPB_TornadoEnhanced::SpawnSplitProjectile(
	const FVector& Direction,
	float RemainingLife,
	float NextRadiusScale)
{
	AMSBaseProjectile* OwnerProj = GetOwnerActor();
	if (!OwnerProj)
	{
		return;
	}

	if (!OwnerProj->ProjectileDataClass)
	{
		return;
	}

	FProjectileRuntimeData NewRuntime = RuntimeData;
	NewRuntime.Direction = Direction;
	NewRuntime.Radius = RuntimeData.Radius * NextRadiusScale;
	if (RemainingLife > 0.f)
	{
		NewRuntime.LifeTime = RemainingLife;
	}

	const FVector SpawnLoc = OwnerProj->GetActorLocation();
	const FTransform SpawnTM(Direction.Rotation(), SpawnLoc);

	UMSFunctionLibrary::LaunchProjectile(
		OwnerProj,
		OwnerProj->ProjectileDataClass,
		NewRuntime,
		SpawnTM,
		OwnerProj->GetOwner(),
		OwnerProj->GetInstigator()
	);
}


