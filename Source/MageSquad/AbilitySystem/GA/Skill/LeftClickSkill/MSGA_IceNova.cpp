// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/GA/Skill/LeftClickSkill/MSGA_IceNova.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "MSGameplayTags.h"
#include "MSFunctionLibrary.h"
#include "Actors/Projectile/Behaviors/MSProjectileBehavior_AreaInstant.h"

UMSGA_IceNova::UMSGA_IceNova()
{
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(MSGameplayTags::Skill_Ability_IceNova);
	SetAssetTags(TagContainer);

	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = FGameplayTag(MSGameplayTags::Skill_Event_IceNova);
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
}

void UMSGA_IceNova::ApplyCooldown(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (!CooldownGameplayEffectClass || !ActorInfo)
	{
		return;
	}

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC)
	{
		return;
	}

	const FGameplayEffectSpecHandle SpecHandle =
		MakeOutgoingGameplayEffectSpec(CooldownGameplayEffectClass, GetAbilityLevel(Handle, ActorInfo));

	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetDuration(CoolTime, true);
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

void UMSGA_IceNova::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	SkillID = CurrentSkillID;
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	CoolTime = SkillListRow.CoolTime;
	Range = SkillListRow.Range;

	AActor* Avatar = GetAvatarActorFromActorInfo();
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

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FProjectileRuntimeData RuntimeData = UMSFunctionLibrary::MakeProjectileRuntimeData(ProjectileDataClass);
	RuntimeData.Damage = SkillListRow.SkillDamage;
	RuntimeData.DamageEffect = DamageEffect;
	RuntimeData.LifeTime = 5.f;
	RuntimeData.Radius = Range;
	RuntimeData.BehaviorClass = UMSProjectileBehavior_AreaInstant::StaticClass();

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

