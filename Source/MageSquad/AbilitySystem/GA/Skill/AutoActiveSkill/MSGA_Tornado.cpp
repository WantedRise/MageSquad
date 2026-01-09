// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GA/Skill/AutoActiveSkill/MSGA_Tornado.h"

#include "MSFunctionLibrary.h"
#include "MSGameplayTags.h"
#include "Actors/Projectile/Behaviors/MSPB_Tornado.h"
#include "Actors/Projectile/Behaviors/MSPB_TornadoEnhanced.h"
#include "Types/MageSquadTypes.h"

UMSGA_Tornado::UMSGA_Tornado()
{
	// 어빌리티 태그 설정
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(MSGameplayTags::Skill_Ability_Tornado);
	SetAssetTags(TagContainer);

	// 트리거 이벤트 태그 설정 (Gameplay Event로 활성화)
	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = FGameplayTag(MSGameplayTags::Skill_Event_Tornado);
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
}

void UMSGA_Tornado::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	SkillID = CurrentSkillID;
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	SkillDamage = SkillListRow.SkillDamage;
	SkillRadius = SkillListRow.Range;
	SkillDuration = SkillListRow.Duration;
	
	UE_LOG(LogTemp, Warning, TEXT("[Tornado GA] ActivateAbility CALLED Auth=%d"), 
	ActorInfo && ActorInfo->IsNetAuthority());
	
	UWorld* World = GetWorld();
	AActor* Avatar = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
	if (!World || !Avatar)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	FProjectileRuntimeData RuntimeData = UMSFunctionLibrary::MakeProjectileRuntimeData(ProjectileDataClass);
	RuntimeData.Damage = SkillDamage;
	RuntimeData.DamageEffect = DamageEffect;
	RuntimeData.Effects = AdditionalEffects;
	RuntimeData.LifeTime = SkillDuration;
	RuntimeData.Radius = SkillRadius;
	RuntimeData.DamageInterval = 0.5f;
	RuntimeData.BehaviorClass = bIsEnhanced ? UMSPB_TornadoEnhanced::StaticClass() : UMSPB_Tornado::StaticClass();
	ApplyPlayerCritToRuntimeData(ActorInfo, RuntimeData);
	
	// 2) 무작위 방향 생성 (XY 평면)
	const float AngleRad = FMath::FRandRange(0.f, 2.f * PI);
	FVector RandDir(
		FMath::Cos(AngleRad),
		FMath::Sin(AngleRad),
		0.f
	);
	RandDir = RandDir.GetSafeNormal();
	
	const FVector SpawnLoc = Avatar->GetActorLocation();
	const FTransform SpawnTM(RandDir.Rotation(), SpawnLoc);
	
	UMSFunctionLibrary::LaunchProjectile(
		this,
		ProjectileDataClass,
		RuntimeData,
		SpawnTM,
		Avatar,
		Cast<APawn>(Avatar)
	);
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
