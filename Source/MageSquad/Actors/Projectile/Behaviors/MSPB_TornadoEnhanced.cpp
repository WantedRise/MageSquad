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
#include "Math/RandomStream.h"

void UMSPB_TornadoEnhanced::OnBegin_Implementation()
{
	AMSBaseProjectile* OwnerProj = GetOwnerActor();
	if (!OwnerProj)
	{
		return;
	}

	bEnded = false;
	CurrentPathIndex = 0;
	LastPathCount = 0;
	bHasTarget = false;

	if (OwnerProj->IsClientSimEnabled())
	{
		StartLocation = OwnerProj->GetClientSimStartLocation();
		ForwardDir = OwnerProj->GetClientSimDirection().GetSafeNormal();
		StartTime = OwnerProj->GetClientSimStartTime();
	}
	else
	{
		StartLocation = OwnerProj->GetActorLocation();
		ForwardDir = OwnerProj->GetActorForwardVector().GetSafeNormal();
		if (UWorld* World = OwnerProj->GetWorld())
		{
			StartTime = World->GetTimeSeconds();
		}
	}
	if (RuntimeData.SFX.IsValidIndex(0) && RuntimeData.SFX[0])
	{
		LoopingSFX = UGameplayStatics::SpawnSoundAttached(RuntimeData.SFX[0], OwnerProj->GetRootComponent());
	}

	if (OwnerProj->HasAuthority())
	{
		bPathStreamInit = true;
		PathStream.Initialize(OwnerProj->GetClientSimNoiseSeed());
		OwnerProj->ClearSimPathPoints();
		OwnerProj->AddSimPathPoint(GenerateNextTarget(StartLocation));
	}

	StartMove();
	if (OwnerProj->HasAuthority())
	{
		StartPeriodicDamage();
		StartSplit();
	}
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
	if (!OwnerProj || !World)
	{
		return;
	}

	const TArray<FVector_NetQuantize>& Points = OwnerProj->GetSimPathPoints();
	if (Points.Num() <= 0)
	{
		return;
	}

	if (Points.Num() != LastPathCount && CurrentPathIndex >= Points.Num())
	{
		CurrentPathIndex = Points.Num() - 1;
		bHasTarget = false;
	}
	LastPathCount = Points.Num();

	if (!bHasTarget)
	{
		CurrentPathIndex = FMath::Clamp(CurrentPathIndex, 0, Points.Num() - 1);
		CurrentTarget = FVector(Points[CurrentPathIndex]);
		bHasTarget = true;
	}

	const int32 PendingPoints = Points.Num() - CurrentPathIndex;
	const float SpeedScale = (PendingPoints >= 2) ? 1.5f : 1.f;
	const float Step = MoveSpeed * SpeedScale * 0.016f;

	const FVector CurrentLoc = OwnerProj->GetActorLocation();
	const FVector ToTarget = CurrentTarget - CurrentLoc;
	const float Dist = ToTarget.Size();

	if (Dist <= Step)
	{
		OwnerProj->SetActorLocation(CurrentTarget, true);
		CurrentPathIndex++;
		bHasTarget = false;

		if (OwnerProj->HasAuthority())
		{
			OwnerProj->AddSimPathPoint(GenerateNextTarget(CurrentTarget));
		}
		return;
	}

	const FVector NewLoc = CurrentLoc + (ToTarget / Dist) * Step;
	OwnerProj->SetActorLocation(NewLoc, true);
}

FVector UMSPB_TornadoEnhanced::GenerateNextTarget(const FVector& From)
{
	AMSBaseProjectile* OwnerProj = GetOwnerActor();
	if (!OwnerProj)
	{
		return From;
	}

	if (!bPathStreamInit)
	{
		bPathStreamInit = true;
		PathStream.Initialize(OwnerProj->GetClientSimNoiseSeed());
	}

	const float TurnInterval = (SwirlFreq > 0.f) ? (PI / SwirlFreq) : 0.3f;
	const float StepDistance = MoveSpeed * TurnInterval;

	const FVector Forward = ForwardDir.GetSafeNormal();
	const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();
	const float Lateral = PathStream.FRandRange(-SwirlAmp, SwirlAmp);
	const float Noise = PathStream.FRandRange(-NoiseAmp, NoiseAmp);

	return From + (Forward * StepDistance) + (Right * (Lateral + Noise));
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


