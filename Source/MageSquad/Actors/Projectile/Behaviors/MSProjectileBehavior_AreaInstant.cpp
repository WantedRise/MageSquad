// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Projectile/Behaviors/MSProjectileBehavior_AreaInstant.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "MSGameplayTags.h"
#include "AbilitySystem/Globals/MSAbilitySystemGlobals.h"
#include "Actors/Projectile/MSBaseProjectile.h"
#include "Engine/OverlapResult.h"

void UMSProjectileBehavior_AreaInstant::OnBegin_Implementation()
{
	if (!IsAuthority() || bDamageApplied)
	{
		return;
	}

	UWorld* World = GetWorldSafe();
	AMSBaseProjectile* OwnerActor = GetOwnerActor();
	if (!World || !OwnerActor)
	{
		return;
	}

	// 스폰 순간 범위
	const float Radius = RuntimeData.Radius;
	const FVector Origin = OwnerActor->GetActorLocation();

	FCollisionObjectQueryParams ObjParams;
	ObjParams.AddObjectTypesToQuery(ECC_GameTraceChannel3); // MSEnemy

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AreaInstant_InitialQuery), false);
	QueryParams.AddIgnoredActor(OwnerActor);

	TArray<FOverlapResult> Hits;
	const bool bAnyHit = World->OverlapMultiByObjectType(
		Hits,
		Origin,
		FQuat::Identity,
		ObjParams,
		FCollisionShape::MakeSphere(Radius),
		QueryParams
	);

	if (bAnyHit)
	{
		for (const FOverlapResult& H : Hits)
		{
			AActor* Target = H.GetActor();
			if (!Target) continue;

			// 중복 방지(컴포넌트 여러개로 중복 히트될 수 있음)
			if (HitActors.Contains(Target)) continue;
			HitActors.Add(Target);

			ApplyDamageToTarget(Target, RuntimeData.Damage);
			if (RuntimeData.Effects.Num() > 0)
			{
				for (const TSubclassOf<UGameplayEffect>& ExtraEffect : RuntimeData.Effects)
				{
					if (!ExtraEffect)
					{
						continue;
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
						continue;
					}

					FGameplayEffectContextHandle Context = TargetASC->MakeEffectContext();
					if (AMSBaseProjectile* OwnerProj = GetOwnerActor())
					{
						Context.AddSourceObject(OwnerProj);
					}

					FGameplayEffectSpecHandle SpecHandle =
						TargetASC->MakeOutgoingSpec(ExtraEffect, 1.f, Context);
					if (!SpecHandle.IsValid())
					{
						continue;
					}

					TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				}
			}
		}
	}

	// 데미지는 끝
	bDamageApplied = true;

	// 추가 판정 차단
	OwnerActor->EnableCollision(false);
}

void UMSProjectileBehavior_AreaInstant::OnTargetEnter_Implementation(AActor* Target, const FHitResult& HitResult)
{
	// 즉발형: 이후 진입 데미지 없음
	(void)Target; (void)HitResult;
}

void UMSProjectileBehavior_AreaInstant::OnEnd_Implementation()
{
	HitActors.Reset();
}

void UMSProjectileBehavior_AreaInstant::ApplyDamageToTarget(AActor* Target, float DamageAmount)
{
	if (!Target || !RuntimeData.DamageEffect)
	{
		return;
	}

	// Target ASC 찾기
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

	// Spec 생성
	FGameplayEffectContextHandle Context = TargetASC->MakeEffectContext();
	if (AMSBaseProjectile* OwnerProj = GetOwnerActor())
	{
		Context.AddSourceObject(OwnerProj);
	}

	FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(RuntimeData.DamageEffect, 1.f, Context);
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

	// 타겟에게 적용
	TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}
