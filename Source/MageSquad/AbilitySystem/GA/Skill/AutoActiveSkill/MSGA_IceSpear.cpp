// Fill out your copyright notice in the Description page of Project Settings.


#include "MSGA_IceSpear.h"
#include "Engine/EngineTypes.h"
#include "Engine/OverlapResult.h"
#include "MSGameplayTags.h"
#include "MSFunctionLibrary.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Actors/Projectile/Behaviors/MSPB_IceSpear.h"
#include "Actors/Projectile/Behaviors/MSProjectileBehavior_Normal.h"

UMSGA_IceSpear::UMSGA_IceSpear()
{
	// 어빌리티 태그 설정
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(MSGameplayTags::Skill_Ability_IceSpear);
	SetAssetTags(TagContainer);

	// 트리거 이벤트 태그 설정 (Gameplay Event로 활성화)
	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = FGameplayTag(MSGameplayTags::Skill_Event_IceSpear);
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
}

void UMSGA_IceSpear::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                     const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                     const FGameplayEventData* TriggerEventData)
{
	SkillID = CurrentSkillID;
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	SkillDamage = SkillListRow.SkillDamage;
	ProjectileNumber = SkillListRow.ProjectileNumber;
	Penetration = SkillListRow.Penetration;
	
	// 가장 가까운 몬스터 찾기
	UWorld* World = GetWorld();
	AActor* Avatar = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
	if (!World || !Avatar)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] World or Avatar is null"), *GetName());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	// 기준점 위치 (플레이어 위치)
	const FVector Origin = Avatar->GetActorLocation();
	FVector TargetLoc = FindClosestEnemyLocation(World, Avatar);
	
	// 근처에 적이 없으거나 투사체 개수가 없으면 어빌리티 종료
	if (TargetLoc.IsZero() || ProjectileNumber <= 0)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// ===== 발사 데이터 캐싱 =====
	CachedOrigin = Origin;
	CachedDirection = (TargetLoc - Origin);
	CachedDirection.Z = 0;
	CachedDirection = CachedDirection.GetSafeNormal();

	CachedSpawnTransform = FTransform(
		CachedDirection.Rotation(),
		CachedOrigin
	);

	CachedRuntimeData = UMSFunctionLibrary::MakeProjectileRuntimeData(ProjectileDataClass);
	CachedRuntimeData.Direction = CachedDirection;
	
	CachedAvatar = Avatar;
	FiredCount = 0;
	
	// 스킬 레벨 별 데이터 적용
	CachedRuntimeData.Damage = SkillDamage;
	CachedRuntimeData.LifeTime = 3.0f;
	CachedRuntimeData.PenetrationCount = Penetration;
	CachedRuntimeData.BehaviorClass = bIsEnhanced
		? UMSPB_IceSpear::StaticClass()
		: UMSProjectileBehavior_Normal::StaticClass();
	CachedRuntimeData.DamageEffect = DamageEffect;
	CachedRuntimeData.Effects = AdditionalEffects;
	ApplyPlayerCritToRuntimeData(ActorInfo, CachedRuntimeData);
	UE_LOG(LogTemp, Log, TEXT("[IceSpear] Lv=%d Damage=%.2f Pen=%d Proj=%d"),
		SkillListRow.SkillLevel,
		CachedRuntimeData.Damage,
		CachedRuntimeData.PenetrationCount,
		ProjectileNumber);
	// ===== 첫 발 발사 =====
	FireNextProjectile();
}

void UMSGA_IceSpear::FireNextProjectile()
{
	// 이미 예약된 Delay Task가 남아있으면(중복 방지) 정리
	if (FireDelayTask)
	{
		FireDelayTask->EndTask();
		FireDelayTask = nullptr;
	}

	// 끝 조건
	if (FiredCount >= ProjectileNumber)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	// 안전 체크
	if (!CachedAvatar.IsValid())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// 1발 발사
	UMSFunctionLibrary::LaunchProjectile(
		this,
		ProjectileDataClass,
		CachedRuntimeData,
		CachedSpawnTransform,
		CachedAvatar.Get(),
		Cast<APawn>(CachedAvatar.Get())
	);

	FiredCount++;

	// 다음 발이 남아 있으면 Delay 예약
	if (FiredCount < ProjectileNumber)
	{
		FireDelayTask = UAbilityTask_WaitDelay::WaitDelay(this, 0.1f);
		if (FireDelayTask)
		{
			FireDelayTask->OnFinish.AddDynamic(this, &UMSGA_IceSpear::FireNextProjectile);
			FireDelayTask->ReadyForActivation();
		}
	}
	else
	{
		// 마지막 발사까지 끝났으면 여기서 종료해도 OK
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}


FVector UMSGA_IceSpear::FindClosestEnemyLocation(const UWorld* World, const AActor* Avatar) const
{
	if (!World || !Avatar)
	{
		return FVector::ZeroVector;
	}

	const FVector Origin = Avatar->GetActorLocation();
	constexpr float SearchRadius = 1500.f;

	// MSEnemy가 ECC_GameTraceChannel3
	FCollisionObjectQueryParams ObjParams;
	ObjParams.AddObjectTypesToQuery(ECC_GameTraceChannel3); // MSEnemy

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(IceSpearFindTarget), /*bTraceComplex=*/false);
	QueryParams.AddIgnoredActor(Avatar);

	TArray<FOverlapResult> Hits;
	const bool bAnyHit = World->OverlapMultiByObjectType(
		Hits,
		Origin,
		FQuat::Identity,
		ObjParams,
		FCollisionShape::MakeSphere(SearchRadius),
		QueryParams
	);

	const AActor* BestTarget = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();

	if (bAnyHit)
	{
		for (const FOverlapResult& H : Hits)
		{
			AActor* Candidate = H.GetActor();
			if (!Candidate) continue;

			const float DistSq = FVector::DistSquared(Origin, Candidate->GetActorLocation());
			if (DistSq < BestDistSq)
			{
				BestDistSq = DistSq;
				BestTarget = Candidate;
			}
		}
	}

	if (!BestTarget)
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] No Enemy target in radius %.1f"), *GetName(), SearchRadius);
		return FVector::ZeroVector;
	}

	return BestTarget->GetActorLocation();
}

