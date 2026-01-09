// Fill out your copyright notice in the Description page of Project Settings.

#include "MSGA_PoisonPlant.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "MSGameplayTags.h"
#include "Player/MSPlayerController.h"
#include "Actors/Projectile/Behaviors/MSPB_AreaPeriodic.h"
#include "MSFunctionLibrary.h"

UMSGA_PoisonPlant::UMSGA_PoisonPlant()
{
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(MSGameplayTags::Skill_Ability_PoisonPlant);
	SetAssetTags(TagContainer);

	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = FGameplayTag(MSGameplayTags::Skill_Event_PoisonPlant);
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
}

void UMSGA_PoisonPlant::ApplyCooldown(
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

void UMSGA_PoisonPlant::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	SkillID = CurrentSkillID;
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	CoolTime = SkillListRow.CoolTime;
	Range = SkillListRow.Range;
	UE_LOG(LogTemp, Log, TEXT("[%s] PoisonPlant Range=%.2f (SkillID=%d)"), *GetName(), Range, SkillID);
	TArray<float> FinalDamageSequence = BaseDamageSequence;
	for (float& Damage : FinalDamageSequence)
	{
		Damage *= SkillListRow.SkillDamage;
	}

	AActor* Avatar = GetAvatarActorFromActorInfo();
	AMSPlayerController* PC = Cast<AMSPlayerController>(ActorInfo->PlayerController.Get());
	if (!Avatar || !PC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FVector PoisonPlantLocation = PC->GetServerCursor();

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

	const FTransform SpawnTM(FRotator::ZeroRotator, PoisonPlantLocation);
	RuntimeData.Radius = Range;
	RuntimeData.LifeTime = 0.f;
	RuntimeData.DamageEffect = DamageEffect;
	RuntimeData.Effects = AdditionalEffects;
	RuntimeData.BehaviorClass = UMSPB_AreaPeriodic::StaticClass();
	RuntimeData.DamageSequence = FinalDamageSequence;
	RuntimeData.DamageInterval = DamageInterval;
	ApplyPlayerCritToRuntimeData(ActorInfo, RuntimeData);

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
