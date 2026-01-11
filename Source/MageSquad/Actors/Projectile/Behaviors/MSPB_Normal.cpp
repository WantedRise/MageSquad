// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Projectile/Behaviors/MSPB_Normal.h"

#include "AbilitySystemComponent.h"
#include "AbilitysystemGlobals.h"
#include "GameplayEffect.h"
#include "MSGameplayTags.h"
#include "Actors/Projectile/MSBaseProjectile.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"

void UMSPB_Normal::OnBegin_Implementation()
{
	HitActors.Reset();

	AMSBaseProjectile* OwnerActor = GetOwnerActor();
	if (!OwnerActor)
	{
		return;
	}

	OwnerActor->PlaySFXAtLocation(0);

	UProjectileMovementComponent* MoveComp = OwnerActor->GetMovementComponent();
	if (!MoveComp)
	{
		return;
	}

	// Movement 세팅
	MoveComp->bInitialVelocityInLocalSpace = false;
	MoveComp->InitialSpeed = RuntimeData.ProjectileSpeed;
	MoveComp->MaxSpeed = RuntimeData.ProjectileSpeed;
	MoveComp->bRotationFollowsVelocity = true;
	MoveComp->bShouldBounce = false;
	MoveComp->Bounciness = 0.f;
	MoveComp->ProjectileGravityScale = 0.f;

	FVector Dir = RuntimeData.Direction.GetSafeNormal();
	if (Dir.IsNearlyZero())
	{
		Dir = OwnerActor->GetActorForwardVector();
	}

	MoveComp->Velocity = RuntimeData.ProjectileSpeed * Dir;

	RemainingPenetration = RuntimeData.PenetrationCount;

}

void UMSPB_Normal::OnTargetEnter_Implementation(
	AActor* Target,
	const FHitResult& HitResult
)
{
	// 서버에서만 피격 처리
	if (!IsAuthority())
	{
		return;
	}

	if (!CanHitTarget(Target))
	{
		return;
	}

	HandleHitTarget(Target, HitResult);

	UE_LOG(LogTemp, Warning, TEXT("[Enter] Target=%s Pen=%d Frame=%llu HitComp=%s"),
		*GetNameSafe(Target),
		RemainingPenetration,
		(uint64)GFrameCounter,
		*GetNameSafe(HitResult.GetComponent()));


	// 데미지 / 치명타 계산
	float FinalDamage = RuntimeData.Damage;
	const bool bIsCritical = FMath::FRand() < RuntimeData.CriticalChance;
	if (bIsCritical)
	{
		FinalDamage *= RuntimeData.CriticalDamage;
	}
	
	// Target ASC
	UAbilitySystemComponent* TargetASC =
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target);
	if (!TargetASC || !RuntimeData.DamageEffect)
	{
		return;
	}
	
	// Context 생성
	FGameplayEffectContextHandle Context = TargetASC->MakeEffectContext();
	
	if (AMSBaseProjectile* OwnerProj = GetOwnerActor())
	{
		Context.AddSourceObject(OwnerProj);
	}
	Context.AddHitResult(HitResult);
	
	// Spec 생성
	FGameplayEffectSpecHandle SpecHandle =
		TargetASC->MakeOutgoingSpec(RuntimeData.DamageEffect, 1.f, Context);
	if (!SpecHandle.IsValid())
	{
		return;
	}
	
	// SetByCaller로 데미지 전달
	SpecHandle.Data->SetSetByCallerMagnitude(
		MSGameplayTags::Data_Damage,
		(FinalDamage * -1.f)
	);
	
	// 치명타 태그 부여
	if (bIsCritical)
	{
		SpecHandle.Data->AddDynamicAssetTag(MSGameplayTags::Hit_Critical);
	}
	
	// GameplayEffect 적용
	TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);

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
	

	AMSBaseProjectile* OwnerActor = GetOwnerActor();
	if (!OwnerActor)
	{
		return;
	}

	// 관통 횟수가 없으면 파괴
	if (RemainingPenetration <= 0)
	{
		OwnerActor->Destroy();
		return;
	}

	// 관통 횟수 소모
	RemainingPenetration--;
}

void UMSPB_Normal::OnEnd_Implementation()
{
	HitActors.Reset();
}

bool UMSPB_Normal::CanHitTarget(AActor* Target) const
{
	if (!IsValid(Target))
	{
		return false;
	}

	const AMSBaseProjectile* OwnerActor = GetOwnerActor();
	if (!OwnerActor)
	{
		return false;
	}

	// 자기 자신 무시
	if (Target == OwnerActor)
	{
		return false;
	}

	// 소유자 / 인스티게이터 무시
	if (Target == OwnerActor->GetOwner())
	{
		return false;
	}

	if (APawn* Inst = OwnerActor->GetInstigator())
	{
		if (Target == Inst)
		{
			return false;
		}
	}

	// 중복 히트 방지
	if (HitActors.Contains(Target))
	{
		return false;
	}

	return true;
}

void UMSPB_Normal::HandleHitTarget(
	AActor* Target,
	const FHitResult& HitResult
)
{
	// 히트 기록
	HitActors.Add(Target);


	(void)HitResult;
}
