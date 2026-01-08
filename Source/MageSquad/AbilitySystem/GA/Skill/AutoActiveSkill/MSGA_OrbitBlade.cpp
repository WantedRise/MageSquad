// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/GA/Skill/AutoActiveSkill/MSGA_OrbitBlade.h"

#include "MSFunctionLibrary.h"
#include "MSGameplayTags.h"
#include "Actors/Projectile/Behaviors/MSProjectileBehavior_OrbitBlade.h"
#include "Player/MSPlayerController.h"
#include "Types/MageSquadTypes.h"

UMSGA_OrbitBlade::UMSGA_OrbitBlade()
{
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(MSGameplayTags::Skill_Ability_OrbitBlade);
	SetAssetTags(TagContainer);

	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = FGameplayTag(MSGameplayTags::Skill_Event_OrbitBlade);
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
}

void UMSGA_OrbitBlade::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	SkillID = CurrentSkillID;
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	Range = SkillListRow.Range > 0.f ? SkillListRow.Range : Range;
	Duration = SkillListRow.Duration > 0.f ? SkillListRow.Duration : Duration;
	const int32 ProjectileNum = SkillListRow.ProjectileNumber > 0 ? SkillListRow.ProjectileNumber : 1;

	AActor* Avatar = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
	AMSPlayerController* PC = ActorInfo ? Cast<AMSPlayerController>(ActorInfo->PlayerController.Get()) : nullptr;
	if (!Avatar || !PC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FProjectileRuntimeData RuntimeData = UMSFunctionLibrary::MakeProjectileRuntimeData(ProjectileDataClass);
	RuntimeData.Damage = SkillListRow.SkillDamage;
	RuntimeData.DamageEffect = DamageEffect;
	RuntimeData.Effects = AdditionalEffects;
	RuntimeData.LifeTime = Duration;
	RuntimeData.Radius = Range;
	RuntimeData.ProjectileSpeed = OrbitSpeedDeg;
	RuntimeData.BehaviorClass = UMSProjectileBehavior_OrbitBlade::StaticClass();
	ApplyPlayerCritToRuntimeData(ActorInfo, RuntimeData);

	const FVector Forward = Avatar->GetActorForwardVector();
	const float StepAngle = 360.f / FMath::Max(1, ProjectileNum);

	for (int32 i = 0; i < ProjectileNum; ++i)
	{
		const float Angle = StepAngle * i;
		const FVector Dir = FRotator(0.f, Angle, 0.f).RotateVector(Forward);
		RuntimeData.Direction = Dir;

		const FTransform SpawnTM(Dir.Rotation(), Avatar->GetActorLocation());

		UMSFunctionLibrary::LaunchProjectile(
			this,
			ProjectileDataClass,
			RuntimeData,
			SpawnTM,
			Avatar,
			Cast<APawn>(Avatar)
		);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
