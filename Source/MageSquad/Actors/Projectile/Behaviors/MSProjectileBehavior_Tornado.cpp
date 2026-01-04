// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Projectile/Behaviors/MSProjectileBehavior_Tornado.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "MSGameplayTags.h"
#include "AbilitySystem/Globals/MSAbilitySystemGlobals.h"
#include "Actors/Projectile/MSBaseProjectile.h"

void UMSProjectileBehavior_Tornado::OnBegin_Implementation()
{
	// 서버에서만
	AMSBaseProjectile* OwnerProj = GetOwnerActor();
	if (!OwnerProj || !OwnerProj->HasAuthority())
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

	StartMove();
	StartPeriodicDamage();
}

void UMSProjectileBehavior_Tornado::OnEnd_Implementation()
{
	if (bEnded)
	{
		return;
	}
	bEnded = true;

	StopMove();
	StopPeriodicDamage();
}

void UMSProjectileBehavior_Tornado::StartMove()
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
			&UMSProjectileBehavior_Tornado::TickMove,
			0.016f,
			true
		);
	}
}

void UMSProjectileBehavior_Tornado::StopMove()
{
	if (UWorld* World = GetWorldSafe())
	{
		World->GetTimerManager().ClearTimer(MoveTimerHandle);
	}
}

void UMSProjectileBehavior_Tornado::TickMove()
{
	AMSBaseProjectile* OwnerProj = GetOwnerActor();
	UWorld* World = GetWorldSafe();
	if (!OwnerProj || !World || !OwnerProj->HasAuthority())
	{
		return;
	}

	const float Now = World->GetTimeSeconds();
	const float T = Now - StartTime;

	// 기본 전진
	const FVector Base = StartLocation + ForwardDir * (MoveSpeed * T);

	// 흔들림 축
	const FVector Right = OwnerProj->GetActorRightVector();
	const FVector Up = OwnerProj->GetActorUpVector();

	// 규칙적 소용돌이 + 불규칙 노이즈
	const float S1 = FMath::Sin(T * SwirlFreq);
	const float S2 = FMath::Cos(T * SwirlFreq * 0.9f);
	const float N  = FMath::PerlinNoise1D(T * NoiseFreq);

	const FVector Offset =
		Right * (S1 * SwirlAmp + N * NoiseAmp) +
		Up    * (S2 * (SwirlAmp * 0.25f));

	const FVector NewLoc = Base + Offset;

	OwnerProj->SetActorLocation(NewLoc, true);
}

void UMSProjectileBehavior_Tornado::StartPeriodicDamage()
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
			&UMSProjectileBehavior_Tornado::TickPeriodicDamage,
			RuntimeData.DamageInterval, // 예: 0.25
			true
		);
	}
}

void UMSProjectileBehavior_Tornado::StopPeriodicDamage()
{
	if (UWorld* World = GetWorldSafe())
	{
		World->GetTimerManager().ClearTimer(DamageTimerHandle);
	}
}

void UMSProjectileBehavior_Tornado::TickPeriodicDamage()
{
	AMSBaseProjectile* OwnerProj = GetOwnerActor();
	if (!OwnerProj || !OwnerProj->HasAuthority())
	{
		OnEnd_Implementation();
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

void UMSProjectileBehavior_Tornado::ApplyDamageToTarget(AActor* Target, float DamageAmount)
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
	SpecHandle.Data->SetSetByCallerMagnitude(MSGameplayTags::Data_Damage, (DamageAmount * -1.f));

	TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}
