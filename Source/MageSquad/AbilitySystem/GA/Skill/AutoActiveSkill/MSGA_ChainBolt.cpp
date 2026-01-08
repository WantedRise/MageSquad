// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/GA/Skill/AutoActiveSkill/MSGA_ChainBolt.h"

#include "MSFunctionLibrary.h"
#include "MSGameplayTags.h"
#include "Actors/Projectile/Behaviors/MSProjectileBehavior_ChainBolt.h"
#include "Types/MageSquadTypes.h"

UMSGA_ChainBolt::UMSGA_ChainBolt()
{
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(MSGameplayTags::Skill_Ability_ChainBolt);
	SetAssetTags(TagContainer);

	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = FGameplayTag(MSGameplayTags::Skill_Event_ChainBolt);
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
}

void UMSGA_ChainBolt::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	SkillID = CurrentSkillID;
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	Range = SkillListRow.Range > 0.f ? SkillListRow.Range : Range;

	AActor* Avatar = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
	if (!Avatar)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!Avatar->HasAuthority())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	if (!ProjectileDataClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] ProjectileDataClass is null"), *GetName());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FProjectileRuntimeData RuntimeData = UMSFunctionLibrary::MakeProjectileRuntimeData(ProjectileDataClass);
	RuntimeData.Damage = SkillListRow.SkillDamage;
	RuntimeData.DamageEffect = DamageEffect;
	RuntimeData.Effects = AdditionalEffects;
	RuntimeData.Radius = Range;
	RuntimeData.ProjectileSpeed = 2000.f;
	RuntimeData.LifeTime = 3.f;
	RuntimeData.BehaviorClass = UMSProjectileBehavior_ChainBolt::StaticClass();
	ApplyPlayerCritToRuntimeData(ActorInfo, RuntimeData);

	const FTransform SpawnTransform(FRotator::ZeroRotator, Avatar->GetActorLocation());

	UMSFunctionLibrary::LaunchProjectile(
		this,
		ProjectileDataClass,
		RuntimeData,
		SpawnTransform,
		Avatar,
		Cast<APawn>(Avatar)
	);

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
