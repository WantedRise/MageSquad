// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/TargetActor/MSTargetActor_IndicatorBase.h"

#include "AbilitySystemComponent.h"

AMSTargetActor_IndicatorBase::AMSTargetActor_IndicatorBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false; // 서버에서만 동작
}

void AMSTargetActor_IndicatorBase::InitializeFromIndicator(
	const FAttackIndicatorParams& Params,
	UAbilitySystemComponent* SourceASC,
	TSubclassOf<UGameplayEffect> InDamageEffectClass)
{
	CachedParams = Params;
	SourceAbilitySystemComponent = SourceASC;
	DamageEffectClass = InDamageEffectClass;
}

TArray<AActor*> AMSTargetActor_IndicatorBase::PerformTargeting()
{
	return PerformOverlapCheck();
}

void AMSTargetActor_IndicatorBase::ApplyDamageToTargets(const TArray<AActor*>& Targets)
{
	if (!SourceAbilitySystemComponent.IsValid() || !DamageEffectClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("MSTargetActor_IndicatorBase: Cannot apply damage - SourceASC or DamageEffect is invalid"));
		return;
	}

	for (AActor* Target : Targets)
	{
		if (!Target)
		{
			continue;
		}

		UAbilitySystemComponent* TargetASC = Target->FindComponentByClass<UAbilitySystemComponent>();
		if (!TargetASC)
		{
			continue;
		}

		// GameplayEffect 적용
		FGameplayEffectContextHandle EffectContext = SourceAbilitySystemComponent->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = SourceAbilitySystemComponent->MakeOutgoingSpec(
			DamageEffectClass,
			1.f,
			EffectContext);

		if (SpecHandle.IsValid())
		{
			SourceAbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
		}
	}
}

TArray<AActor*> AMSTargetActor_IndicatorBase::ExecuteTargetingAndDamage()
{
	TArray<AActor*> HitTargets = PerformTargeting();
	ApplyDamageToTargets(HitTargets);
	return HitTargets;
}

bool AMSTargetActor_IndicatorBase::IsValidTarget(AActor* Actor) const
{
	if (!Actor)
	{
		return false;
	}

	// 자기 자신 제외
	if (Actor == GetOwner())
	{
		return false;
	}

	// 플레이어 태그 체크
	if (!Actor->ActorHasTag(PlayerTag))
	{
		return false;
	}

	return true;
}
