// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GA/AutoActiveSkill/MSGA_IceSpear.h"
#include "Engine/EngineTypes.h"
#include "Engine/OverlapResult.h"

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

	const FVector TargetLoc = BestTarget->GetActorLocation();
	UE_LOG(LogTemp, Log, TEXT("[%s] Target=%s Loc=%s Dist=%.1f"),
		*GetName(),
		*BestTarget->GetName(),
		*TargetLoc.ToString(),
		FMath::Sqrt(BestDistSq)
	);
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
