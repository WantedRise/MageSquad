// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Projectile/Behaviors/MSProjectileBehavior_AreaInstant.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "MSGameplayTags.h"
#include "AbilitySystem/Globals/MSAbilitySystemGlobals.h"
#include "Actors/Projectile/MSBaseProjectile.h"

void UMSProjectileBehavior_AreaInstant::OnBegin_Implementation()
{
	HitActors.Reset();
}

void UMSProjectileBehavior_AreaInstant::OnTargetEnter_Implementation(AActor* Target, const FHitResult& HitResult)
{
	(void)HitResult;

	if (!IsAuthority() || !Target)
	{
		return;
	}

	// 중복 방지
	if (HitActors.Contains(Target))
	{
		return;
	}
	HitActors.Add(Target);

	const FProjectileRuntimeData& RuntimeData = GetRuntimeData();
	ApplyDamageToTarget(Target, RuntimeData.Damage);
}

void UMSProjectileBehavior_AreaInstant::OnEnd_Implementation()
{
	HitActors.Reset();
}

void UMSProjectileBehavior_AreaInstant::ApplyDamageToTarget(AActor* Target, float DamageAmount)
{
	const FProjectileRuntimeData& RuntimeData = GetRuntimeData();
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

	SpecHandle.Data->SetSetByCallerMagnitude(MSGameplayTags::Data_Damage, (DamageAmount * -1.f));

	// 타겟에게 적용
	TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}
