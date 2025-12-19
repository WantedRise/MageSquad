// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GA/Shared/MSGA_DamageFloater.h"

#include "AbilitySystemComponent.h"
#include "GameplayTagsManager.h"

#include "MSGameplayTags.h"

UMSGA_DamageFloater::UMSGA_DamageFloater()
{
	// 서버에서 CUe 실행 -> 관련 클라이언트에서 CueNotify가 실행되어 로컬 UI를 띄우는 구조
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// 어빌리티 태그 설정
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(MSGameplayTags::Shared_Ability_DrawDamageNumber);
	SetAssetTags(TagContainer);

	// 트리거 이벤트 태그 설정 (Gameplay Event로 활성화)
	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = FGameplayTag(MSGameplayTags::Shared_Event_DrawDamageNumber);
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);

	// GameplayCue 태그 설정
	const UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
	Cue_DamageFloater = TagsManager.RequestGameplayTag(FName("GameplayCue.UI.DamageFloater"), false);
}

void UMSGA_DamageFloater::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 어빌리티를 활성화해도 되는지 검사
	if (!CheckAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// 소유자 액터 및 ASC 가져오기
	AActor* TargetActor = ActorInfo->AvatarActor.Get();
	UAbilitySystemComponent* TargetASC = ActorInfo->AbilitySystemComponent.Get();
	if (!TargetActor || !TargetASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// 받은 혹은 회복된 수치를 저장 (AttributeSet에서 전달받음)
	const float DeltaHealth = TriggerEventData ? TriggerEventData->EventMagnitude : 0.f;

	// GameplayCue 파라미터 설정
	FGameplayCueParameters Params;
	Params.RawMagnitude = DeltaHealth;
	Params.Location = TargetActor->GetActorLocation();

	// 전달된 태그가 있다면 같이 전달 (ex. 치명타, 화상 등)
	Params.AggregatedSourceTags = TriggerEventData ? TriggerEventData->InstigatorTags : FGameplayTagContainer();
	Params.EffectContext = TriggerEventData ? TriggerEventData->ContextHandle : FGameplayEffectContextHandle();

	// GameplayCue 실행
	TargetASC->ExecuteGameplayCue(Cue_DamageFloater, Params);

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

bool UMSGA_DamageFloater::CheckAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// 액터 유효성 검사
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid()) return false;

	// 서버에서만 로직 수행
	if (!ActorInfo->IsNetAuthority()) return false;

	return true;
}
