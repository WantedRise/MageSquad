// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Projectile/Behaviors/MSPB_AreaPeriodic.h"
#include "Actors/Projectile/MSBaseProjectile.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "MSGameplayTags.h"

void UMSPB_AreaPeriodic::OnBegin_Implementation()
{
	// ?œë²„?ì„œë§?
	AMSBaseProjectile* OwnerProj = GetOwnerActor();
	if (!OwnerProj || !OwnerProj->HasAuthority())
	{
		return;
	}

	StartPeriodicDamage();
}

void UMSPB_AreaPeriodic::OnEnd_Implementation()
{
	StopPeriodicDamage();
}

void UMSPB_AreaPeriodic::StartPeriodicDamage()
{
	if (bRunning)
	{
		return;
	}

	AMSBaseProjectile* OwnerProj = GetOwnerActor();
	if (!OwnerProj)
	{
		return;
	}
	if (RuntimeData.DamageInterval <= 0.f)
	{
		// ì£¼ê¸°ê°€ 0?´ë©´ ë¬´í•œ ?¸ì¶œ ?„í—˜ -> ë°©ì?
		return;
	}

	// ?œí€€?¤ê? ë¹„ì–´?ˆìœ¼ë©?ì¢…ë£Œ
	if (RuntimeData.DamageSequence.Num() <= 0)
	{
		return;
	}

	bRunning = true;
	CurrentTickIndex = 0;

	// ì²???ì¦‰ì‹œ 1???ìš© ?? ?´í›„ ì£¼ê¸° ë°˜ë³µ
	TickPeriodicDamage();

	if (UWorld* World = OwnerProj->GetWorld())
	{
		World->GetTimerManager().SetTimer(
			PeriodicTimerHandle,
			this,
			&UMSPB_AreaPeriodic::TickPeriodicDamage,
			RuntimeData.DamageInterval,
			true
		);
	}
}

void UMSPB_AreaPeriodic::StopPeriodicDamage()
{
	if (!bRunning)
	{
		return;
	}

	bRunning = false;

	if (AMSBaseProjectile* OwnerProj = GetOwnerActor())
	{
		if (UWorld* World = OwnerProj->GetWorld())
		{
			World->GetTimerManager().ClearTimer(PeriodicTimerHandle);
		}
	}
}

void UMSPB_AreaPeriodic::TickPeriodicDamage()
{
	AMSBaseProjectile* OwnerProj = GetOwnerActor();
	if (!OwnerProj || !OwnerProj->HasAuthority())
	{
		StopPeriodicDamage();
		return;
	}

	if (CurrentTickIndex >= RuntimeData.DamageSequence.Num())
	{
		// ëª¨ë“  ???„ë£Œ -> ì¢…ë£Œ
		StopPeriodicDamage();

		return;
	}

	// ?´ë²ˆ ???°ë?ì§€
	const float DamageAmount = RuntimeData.DamageSequence[CurrentTickIndex];
	++CurrentTickIndex;

	// ?¥íŒ ë²”ìœ„ ?ˆì˜ ?¡í„° ?˜ì§‘ (ë£¨íŠ¸=CollisionSphere??ê°€??
	UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(OwnerProj->GetRootComponent());
	if (!RootPrim)
	{
		return;
	}

	TArray<AActor*> OverlappingActors;
	RootPrim->GetOverlappingActors(OverlappingActors);

	// ê°™ì? ?¡í„° ì¤‘ë³µ ë°©ì?
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

void UMSPB_AreaPeriodic::ApplyDamageToTarget(AActor* Target, float DamageAmount)
{
	// ë°œì‚¬ì²?ê°€?¸ì˜¤ê¸?
	AMSBaseProjectile* OwnerProj = GetOwnerActor();
	if (!OwnerProj || !Target)
	{
		return;
	}
	if (!RuntimeData.DamageEffect)
	{
		return;
	}

	// ?€ê²?ASC ê°€?¸ì˜¤ê¸?
	UAbilitySystemComponent* TargetASC =
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target);

	if (!TargetASC)
	{
		return;
	}

	// ?€ê²?ASCë¡?Spec ?ì„±
	FGameplayEffectContextHandle Context = TargetASC->MakeEffectContext();
	Context.AddSourceObject(OwnerProj); // ?œì´ ?°ë?ì§€??ì¶œì²˜ ?¤ë¸Œ?íŠ¸???•ë„ë§??¨ê?(? íƒ)

	FGameplayEffectSpecHandle SpecHandle =
		TargetASC->MakeOutgoingSpec(RuntimeData.DamageEffect, 1.f, Context);

	if (!SpecHandle.IsValid())
	{
		return;
	}

	// SetByCallerë¥??°ëŠ” GE
	// ?„ë¡œ?íŠ¸ ?œê·¸ëª…ì— ë§ê²Œ ë°”ê¿”ì¤?(?? Data_Damage)
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

	// ?€ê²Ÿì—ê²??ìš©
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
