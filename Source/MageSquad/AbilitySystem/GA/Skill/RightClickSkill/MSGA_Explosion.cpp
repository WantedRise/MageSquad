// Fill out your copyright notice in the Description page of Project Settings.


#include "MSGA_Explosion.h"
#include "AbilitySystemComponent.h"
#include "MSGameplayTags.h"
#include "Player/MSPlayerController.h"

UMSGA_Explosion::UMSGA_Explosion()
{
	// 어빌리티 태그 설정
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(MSGameplayTags::Skill_Ability_Explosion);
	SetAssetTags(TagContainer);

	// 트리거 이벤트 태그 설정 (Gameplay Event로 활성화)
	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = FGameplayTag(MSGameplayTags::Skill_Event_Explosion);
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
}

void UMSGA_Explosion::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	SkillID = CurrentSkillID;
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	SkillDamage = SkillDataRow.SkillDamage;
	CoolTime = SkillDataRow.CoolTime;
	Range = SkillDataRow.Range;
	
	AActor* Avatar = GetAvatarActorFromActorInfo();
	AMSPlayerController* PC = Cast<AMSPlayerController>(ActorInfo->PlayerController.Get());
	if (!Avatar || !PC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 서버가 알고 있는 커서 위치
	const FVector ExplosionLocation = PC->GetServerCursor();

	FGameplayCueParameters Params;
	Params.Location = ExplosionLocation;
	Params.RawMagnitude = Range;
	
	GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(
		MSGameplayTags::GameplayCue_Skill_Explosion,
		Params
	);
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
