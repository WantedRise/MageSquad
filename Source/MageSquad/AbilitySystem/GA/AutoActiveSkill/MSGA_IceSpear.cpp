// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GA/AutoActiveSkill/MSGA_IceSpear.h"
#include "Engine/EngineTypes.h"
#include "Engine/OverlapResult.h"
#include "MSGameplayTags.h"
#include "MSFunctionLibrary.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"

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
	SkillDamage = SkillDataRow.SkillDamage;
	CoolTime = SkillDataRow.CoolTime;
	ProjectileNumber = SkillDataRow.ProjectileNumber;
	Penetration = SkillDataRow.Penetration;
	
	UE_LOG(LogTemp, Log,
			TEXT("SkillID : %d, Skill Level : %d, Skill Name : %s"), SkillDataRow.SkillID, SkillDataRow.SkillLevel, *SkillDataRow.SkillName);
	
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

	// 사거리
	constexpr float SearchRadius = 1200.f;
	
	// Pawn만 검색
	FCollisionObjectQueryParams ObjParams;
	ObjParams.AddObjectTypesToQuery(ECC_Pawn);
	
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(IceSpearFindTarget), false);
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
	
	AActor* BestTarget = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();
	
	if (bAnyHit)
	{
		for (const FOverlapResult& H : Hits)
		{
			AActor* Candidate = H.GetActor();
			if (!Candidate) continue;
			
			// 몬스터 "Enemy" 태그 검색
			if (!Candidate->ActorHasTag(TEXT("Enemy")))
				continue;

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
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// ===== 발사 데이터 캐싱 =====
	CachedOrigin = Origin;
	CachedDirection = (BestTarget->GetActorLocation() - Origin);
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

	// ===== 첫 발 발사 =====
	FireNextProjectile();
}

void UMSGA_IceSpear::FireNextProjectile()
{
	if (FiredCount >= ProjectileNumber)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	if (!CachedAvatar.IsValid())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	UMSFunctionLibrary::LaunchProjectile(
		this,
		ProjectileDataClass,
		CachedRuntimeData,
		CachedSpawnTransform,
		CachedAvatar.Get(),
		Cast<APawn>(CachedAvatar.Get())
	);

	FiredCount++;

	// 다음 발이 남아 있으면 Delay
	if (FiredCount < ProjectileNumber)
	{
		UAbilityTask_WaitDelay* WaitTask =
			UAbilityTask_WaitDelay::WaitDelay(this, 0.1f);

		WaitTask->OnFinish.AddDynamic(this, &UMSGA_IceSpear::FireNextProjectile);
		WaitTask->ReadyForActivation();
	}
}