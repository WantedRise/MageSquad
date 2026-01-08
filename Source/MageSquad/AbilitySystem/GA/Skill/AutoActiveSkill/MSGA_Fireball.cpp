// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GA/Skill/AutoActiveSkill/MSGA_Fireball.h"

#include "Actors/Projectile/Behaviors/MSProjectileBehavior_Explosive.h"
#include "MSFunctionLibrary.h"
#include "MSGameplayTags.h"
#include "Player/MSPlayerController.h"
#include "Types/MageSquadTypes.h"

UMSGA_Fireball::UMSGA_Fireball()
{
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(MSGameplayTags::Skill_Ability_Fireball);
	SetAssetTags(TagContainer);

	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = FGameplayTag(MSGameplayTags::Skill_Event_Fireball);
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
}

void UMSGA_Fireball::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	SkillID = CurrentSkillID;
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	SkillDamage = SkillListRow.SkillDamage;
	SkillRadius = SkillListRow.Range;

	AActor* Avatar = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
	AMSPlayerController* PC = ActorInfo ? Cast<AMSPlayerController>(ActorInfo->PlayerController.Get()) : nullptr;
	if (!Avatar || !PC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FProjectileRuntimeData RuntimeData = UMSFunctionLibrary::MakeProjectileRuntimeData(ProjectileDataClass);
	RuntimeData.Damage = SkillDamage;
	RuntimeData.DamageEffect = DamageEffect;
	RuntimeData.Effects = AdditionalEffects;
	RuntimeData.Radius = SkillRadius;
	RuntimeData.LifeTime = 5.f;
	RuntimeData.BehaviorClass = UMSProjectileBehavior_Explosive::StaticClass();
	RuntimeData.Direction = PC->GetServerCursorDir(Avatar->GetActorForwardVector());
	ApplyPlayerCritToRuntimeData(ActorInfo, RuntimeData);

	const FVector SpawnLocation = Avatar->GetActorLocation() + FVector(0.f, 0.f, 50.f);
	const FTransform SpawnTransform(RuntimeData.Direction.Rotation(), SpawnLocation);

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
