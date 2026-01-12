// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/GA/Skill/LeftClickSkill/MSGA_SharkFin.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "MSGameplayTags.h"
#include "MSFunctionLibrary.h"
#include "Player/MSPlayerController.h"
#include "Actors/Projectile/Behaviors/MSPB_TrailDoT.h"

UMSGA_SharkFin::UMSGA_SharkFin()
{
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(MSGameplayTags::Skill_Ability_SharkFin);
	SetAssetTags(TagContainer);

	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = FGameplayTag(MSGameplayTags::Skill_Event_SharkFin);
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
}

void UMSGA_SharkFin::ApplyCooldown(
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

void UMSGA_SharkFin::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	SkillID = CurrentSkillID;
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	CoolTime = SkillListRow.CoolTime;
	Range = SkillListRow.Range;

	AActor* Avatar = GetAvatarActorFromActorInfo();
	AMSPlayerController* PC = ActorInfo ? Cast<AMSPlayerController>(ActorInfo->PlayerController.Get()) : nullptr;
	if (!Avatar || !PC)
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
	RuntimeData.DamageInterval = 0.5f;
	RuntimeData.DamageEffect = DamageEffect;
	RuntimeData.Effects = AdditionalEffects;
	RuntimeData.Radius = Range;
RuntimeData.LifeTime = 4.f;
RuntimeData.BehaviorClass = UMSPB_TrailDoT::StaticClass();
RuntimeData.Direction = PC->GetServerCursorDir(Avatar->GetActorForwardVector());
ApplyPlayerCritToRuntimeData(ActorInfo, RuntimeData);

	const FVector SpawnLocation = Avatar->GetActorLocation();
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

