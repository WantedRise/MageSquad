// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Projectile/Behaviors/MSPB_Tornado.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "MSGameplayTags.h"
#include "AbilitySystem/Globals/MSAbilitySystemGlobals.h"
#include "Actors/Projectile/MSBaseProjectile.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/RandomStream.h"
#include "NiagaraComponent.h"
#include "Math/RandomStream.h"

void UMSPB_Tornado::OnBegin_Implementation()
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

	const float VfxScale = (RuntimeData.Radius > 0.f) ? (RuntimeData.Radius / 300.f) : 1.f;
	ApplyVfxScale(VfxScale);

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
	}
}

void UMSPB_Tornado::OnEnd_Implementation()
{
	if (bEnded)
	{
		return;
	}
	bEnded = true;

	StopMove();
	StopPeriodicDamage();
	if (LoopingSFX.IsValid())
	{
		LoopingSFX->Stop();
		LoopingSFX = nullptr;
	}
}

void UMSPB_Tornado::StartMove()
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
			&UMSPB_Tornado::TickMove,
			0.016f,
			true
		);
	}
}

void UMSPB_Tornado::StopMove()
{
	if (UWorld* World = GetWorldSafe())
	{
		World->GetTimerManager().ClearTimer(MoveTimerHandle);
	}
}

void UMSPB_Tornado::TickMove()
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

FVector UMSPB_Tornado::GenerateNextTarget(const FVector& From)
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

void UMSPB_Tornado::ApplyVfxScale(float Scale)
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

void UMSPB_Tornado::StartPeriodicDamage()
{
	AMSBaseProjectile* OwnerProj = GetOwnerActor();
	if (!OwnerProj)
	{
		return;
	}

	// 방어 코드
	if (RuntimeData.DamageInterval <= 0.f)
	{
		return; // 0이면 무한 호출 위험
	}
	if (!RuntimeData.DamageEffect)
	{
		return;
	}

	// 첫 틱 즉시
	TickPeriodicDamage();

	if (UWorld* World = OwnerProj->GetWorld())
	{
		World->GetTimerManager().ClearTimer(DamageTimerHandle);
		World->GetTimerManager().SetTimer(
			DamageTimerHandle,
			this,
			&UMSPB_Tornado::TickPeriodicDamage,
			RuntimeData.DamageInterval, // 예: 0.25
			true
		);
	}
}

void UMSPB_Tornado::StopPeriodicDamage()
{
	if (UWorld* World = GetWorldSafe())
	{
		World->GetTimerManager().ClearTimer(DamageTimerHandle);
	}
}

void UMSPB_Tornado::TickPeriodicDamage()
{
	AMSBaseProjectile* OwnerProj = GetOwnerActor();
	if (!OwnerProj || !OwnerProj->HasAuthority())
	{
		return;
	}

	// 이번 틱 데미지(항상 동일)
	const float DamageAmount = RuntimeData.Damage;

	// 현재 범위 내 타겟 스캔 (Exit 필요 없음)
	UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(OwnerProj->GetRootComponent());
	if (!RootPrim)
	{
		return;
	}

	TArray<AActor*> OverlappingActors;
	RootPrim->GetOverlappingActors(OverlappingActors);

	// 중복 제거
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

void UMSPB_Tornado::ApplyDamageToTarget(AActor* Target, float DamageAmount)
{
	AMSBaseProjectile* OwnerProj = GetOwnerActor();
	if (!OwnerProj || !Target)
	{
		return;
	}

	// 타겟 ASC
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

	// SetByCaller (프로젝트 태그 그대로 사용)
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


