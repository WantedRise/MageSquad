// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GA/Skill/AutoActiveSkill/MSGA_Lightning.h"

#include "MSFunctionLibrary.h"
#include "MSGameplayTags.h"
#include "Actors/Projectile/Behaviors/MSProjectileBehavior_AreaInstant.h"
#include "Engine/OverlapResult.h"
#include "Types/MageSquadTypes.h"

UMSGA_Lightning::UMSGA_Lightning()
{
	// 어빌리티 태그 설정
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(MSGameplayTags::Skill_Ability_Lightning);
	SetAssetTags(TagContainer);

	// 트리거 이벤트 태그 설정 (Gameplay Event로 활성화)
	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = FGameplayTag(MSGameplayTags::Skill_Event_Lightning);
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
}

void UMSGA_Lightning::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	SkillID = CurrentSkillID;
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	SkillDamage = SkillListRow.SkillDamage;
	ProjectileNumber = SkillListRow.ProjectileNumber;
	SkillRadius = SkillListRow.Range;
	
	UE_LOG(LogTemp, Warning, TEXT("[Lightning GA] ActivateAbility CALLED Auth=%d"), 
	ActorInfo && ActorInfo->IsNetAuthority());
	
	AActor* Avatar = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
	const UWorld* World = Avatar ? Avatar->GetWorld() : nullptr;
	if (!Avatar || !World)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	
	TArray<AActor*> Targets;
	FindRandomEnemyTargets(
		World,
		Avatar,
		ProjectileNumber,
		Targets
	);
	
	FProjectileRuntimeData RuntimeData = UMSFunctionLibrary::MakeProjectileRuntimeData(ProjectileDataClass);
	RuntimeData.Damage = SkillDamage;
	RuntimeData.DamageEffect = DamageEffect;
	RuntimeData.LifeTime = 1.f;
	RuntimeData.Radius = SkillRadius;
	RuntimeData.BehaviorClass = UMSProjectileBehavior_AreaInstant::StaticClass();
	
	for (auto& Target : Targets)
	{
		UMSFunctionLibrary::LaunchProjectile(
			this,
			ProjectileDataClass,
			RuntimeData,
			Target->GetActorTransform(),
			Avatar,
			Cast<APawn>(Avatar)
		);
		
		UE_LOG(LogTemp, Warning, TEXT("[Lightning GA] Launch at target=%s"), *GetNameSafe(Target));
	}
	
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UMSGA_Lightning::FindRandomEnemyTargets(const UWorld* World, const AActor* Avatar,
	int32 NumToPick, TArray<AActor*>& OutTargets) const
{
	OutTargets.Reset();

	if (!World || !Avatar || NumToPick <= 0)
	{
		return;
	}

	const FVector Origin = Avatar->GetActorLocation();

	// MSEnemy가 ECC_GameTraceChannel3
	FCollisionObjectQueryParams ObjParams;
	ObjParams.AddObjectTypesToQuery(ECC_GameTraceChannel3);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(LightningFindTargets), false);
	QueryParams.AddIgnoredActor(Avatar);

	TArray<FOverlapResult> Hits;
	const bool bAnyHit = World->OverlapMultiByObjectType(
		Hits,
		Origin,
		FQuat::Identity,
		ObjParams,
		FCollisionShape::MakeSphere(1500.f),
		QueryParams
	);

	if (!bAnyHit)
	{
		return;
	}

	// 후보 수집 + 중복 제거
	TArray<AActor*> Candidates;
	Candidates.Reserve(Hits.Num());

	for (const FOverlapResult& H : Hits)
	{
		AActor* Candidate = H.GetActor();
		if (!Candidate) continue;

		// 필요하면 여기서 살아있는 몬스터만, 팀필터 등 조건 추가
		Candidates.AddUnique(Candidate);
	}

	if (Candidates.Num() <= 0)
	{
		return;
	}

	// NumToPick 만큼 비복원 랜덤 선택
	const int32 PickCount = FMath::Min(NumToPick, Candidates.Num());

	// 간단/빠름: 후보 셔플 후 앞에서 PickCount개
	for (int32 i = Candidates.Num() - 1; i > 0; --i)
	{
		const int32 j = FMath::RandRange(0, i);
		Candidates.Swap(i, j);
	}

	OutTargets.Append(Candidates.GetData(), PickCount);
}
