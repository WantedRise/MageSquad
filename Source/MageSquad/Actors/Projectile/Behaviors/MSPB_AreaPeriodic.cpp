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
	// 서버에서만
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
		// 주기가 0이면 무한 호출 위험 -> 방지
		return;
	}

	// 시퀀스가 비어있으면 종료
	if (RuntimeData.DamageSequence.Num() <= 0)
	{
		return;
	}

	bRunning = true;
	CurrentTickIndex = 0;

	// 첫 틱 즉시 1회 적용 후, 이후 주기 반복
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
		// 모든 틱 완료 -> 종료
		StopPeriodicDamage();
		OwnerProj->Destroy();
		return;
	}

	// 이번 틱 데미지
	const float DamageAmount = RuntimeData.DamageSequence[CurrentTickIndex];
	++CurrentTickIndex;

	// 장판 범위 안의 액터 수집 (루트=CollisionSphere라 가정)
	UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(OwnerProj->GetRootComponent());
	if (!RootPrim)
	{
		return;
	}

	TArray<AActor*> OverlappingActors;
	RootPrim->GetOverlappingActors(OverlappingActors);

	// 같은 액터 중복 방지
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
	// 발사체 가져오기
	AMSBaseProjectile* OwnerProj = GetOwnerActor();
	if (!OwnerProj || !Target)
	{
		return;
	}
	if (!RuntimeData.DamageEffect)
	{
		return;
	}

	// 타겟 ASC 가져오기
	UAbilitySystemComponent* TargetASC =
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target);

	if (!TargetASC)
	{
		return;
	}

	// 타겟 ASC로 Spec 생성
	FGameplayEffectContextHandle Context = TargetASC->MakeEffectContext();
	Context.AddSourceObject(OwnerProj); // “이 데미지의 출처 오브젝트” 정도만 남김(선택)

	FGameplayEffectSpecHandle SpecHandle =
		TargetASC->MakeOutgoingSpec(RuntimeData.DamageEffect, 1.f, Context);

	if (!SpecHandle.IsValid())
	{
		return;
	}

	// SetByCaller를 쓰는 GE
	// 프로젝트 태그명에 맞게 바꿔줘 (예: Data_Damage)
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
