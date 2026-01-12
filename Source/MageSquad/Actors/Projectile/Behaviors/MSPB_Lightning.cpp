// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Projectile/Behaviors/MSPB_Lightning.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/Globals/MSAbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "MSGameplayTags.h"
#include "NiagaraFunctionLibrary.h"
#include "Actors/Projectile/MSBaseProjectile.h"
#include "Engine/OverlapResult.h"
#include "TimerManager.h"

void UMSPB_Lightning::OnBegin_Implementation()
{
	if (!IsAuthority() || bDamageApplied)
	{
		return;
	}

	AMSBaseProjectile* OwnerActor = GetOwnerActor();
	if (!OwnerActor)
	{
		return;
	}

	ImpactOrigin = OwnerActor->GetActorLocation();
	OwnerActor->PlaySFXAtLocation(0);
	ApplyDamageInRadius(RuntimeData.Radius, RuntimeData.Damage, 0.0f);

	bDamageApplied = true;
	OwnerActor->EnableCollision(false);

	if (UWorld* World = GetWorldSafe())
	{
		World->GetTimerManager().SetTimer(
			DelayedStrikeHandle,
			this,
			&UMSPB_Lightning::HandleDelayedStrike,
			0.5f,
			false
		);
	}
}

void UMSPB_Lightning::OnTargetEnter_Implementation(AActor* Target, const FHitResult& HitResult)
{
	(void)Target;
	(void)HitResult;
}

void UMSPB_Lightning::OnEnd_Implementation()
{
	if (UWorld* World = GetWorldSafe())
	{
		World->GetTimerManager().ClearTimer(DelayedStrikeHandle);
	}
}

void UMSPB_Lightning::ApplyDamageInRadius(float Radius, float Damage, float VfxScale)
{
	UWorld* World = GetWorldSafe();
	AMSBaseProjectile* OwnerActor = GetOwnerActor();
	if (!World || !OwnerActor || Radius <= 0.f)
	{
		return;
	}

	if (RuntimeData.OnHitVFX && VfxScale > 0.f)
	{
		OwnerActor->Multicast_SpawnVFXAtLocation(RuntimeData.OnHitVFX, ImpactOrigin, VfxScale);
	}

	FCollisionObjectQueryParams ObjParams;
	ObjParams.AddObjectTypesToQuery(ECC_GameTraceChannel3);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(LightningAreaInstant), false);
	QueryParams.AddIgnoredActor(OwnerActor);

	TArray<FOverlapResult> Hits;
	const bool bAnyHit = World->OverlapMultiByObjectType(
		Hits,
		ImpactOrigin,
		FQuat::Identity,
		ObjParams,
		FCollisionShape::MakeSphere(Radius),
		QueryParams
	);

	if (!bAnyHit)
	{
		return;
	}

	TSet<TWeakObjectPtr<AActor>> HitActors;
	for (const FOverlapResult& H : Hits)
	{
		AActor* Target = H.GetActor();
		if (!Target || HitActors.Contains(Target))
		{
			continue;
		}
		HitActors.Add(Target);

		if (RuntimeData.DamageEffect)
		{
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
				TargetASC->MakeOutgoingSpec(RuntimeData.DamageEffect, 1.f, Context);
			if (!SpecHandle.IsValid())
			{
				continue;
			}

			float FinalDamage = Damage;
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
	}
}

void UMSPB_Lightning::HandleDelayedStrike()
{
	if (!IsAuthority())
	{
		return;
	}

	if (AMSBaseProjectile* OwnerActor = GetOwnerActor())
	{
		OwnerActor->PlaySFXAtLocation(0);
	}

	const float ScaledRadius = RuntimeData.Radius * 1.5f;
	ApplyDamageInRadius(ScaledRadius, RuntimeData.Damage, 1.5f);
}
