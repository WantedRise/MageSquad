// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GA/Enemy/MSGA_Boss_Sevarog_Meteor.h"

#include "AbilitySystemComponent.h"
#include "MSGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/Tasks/MSAT_ChaseAndSpawnMeteor.h"
#include "AbilitySystem/Tasks/MSAT_PlayMontageAndWaitForEvent.h"
#include "Actors/Indicator/MSIndicatorActor.h"
#include "Enemy/MSBossEnemy.h"

UMSGA_Boss_Sevarog_Meteor::UMSGA_Boss_Sevarog_Meteor()
{
	// 어빌리티 식별 Tag
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(MSGameplayTags::Enemy_Ability_Pattern1);
	SetAssetTags(TagContainer);

	// 활성화 시 Owner에게 부여되는 Tag
	ActivationOwnedTags.AddTag(MSGameplayTags::Enemy_State_Pattern1);
	
	// 기본 Indicator 파라미터 설정
	IndicatorParams.Shape = EIndicatorShape::Circle;
	IndicatorParams.Radius = 200.f;
	IndicatorParams.Duration = 1.0f;
}

void UMSGA_Boss_Sevarog_Meteor::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	// CommitAbility 체크 (쿨다운, 코스트 등)
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (UAnimMontage* Pattern1Montage = Owner->GetPattern1Montage())
	{
		// UAbilityTask_PlayMontageAndWait* Pattern1Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("Pattern1"), Pattern1Montage);
		// Pattern1Task->OnCompleted.AddDynamic(this, &UMSGA_Boss_Sevarog_Meteor::OnCompleteCallback); // 몽타주가 끝나면 호출될 함수
		// Pattern1Task->OnInterrupted.AddDynamic(this, &UMSGA_Boss_Sevarog_Meteor::OnInterruptedCallback); // 몽타주가 중단되면 호출될 함수
		// Pattern1Task->ReadyForActivation();
		
		FGameplayTagContainer Tags;
		Tags.AddTag(FGameplayTag::RequestGameplayTag("Enemy.Event.NormalAttack"));
		
		// Todo : 추후에 페이즈 전환 관련 델리게이트로 빼서 관리할 예정
		FName StartSectionName = NAME_None;
		// if (Owner->GetAbilitySystemComponent()->HasMatchingGameplayTag(MSGameplayTags::Enemy_State_Phase2))
		// {
		// 	StartSectionName = TEXT("Phase2");
		// }

		UMSAT_PlayMontageAndWaitForEvent* Pattern1Task = 
			UMSAT_PlayMontageAndWaitForEvent::CreateTask(this, Pattern1Montage, Tags, 1.f, StartSectionName);

		Pattern1Task->OnCompleted.AddDynamic(this, &UMSGA_Boss_Sevarog_Meteor::OnCompleteCallback);
		Pattern1Task->OnInterrupted.AddDynamic(this, &UMSGA_Boss_Sevarog_Meteor::OnInterruptedCallback);
		Pattern1Task->OnEventReceived.AddDynamic(this, &UMSGA_Boss_Sevarog_Meteor::OnEventReceivedCallback);
		Pattern1Task->ReadyForActivation();
	}
	
	// // 서버에서만 추적 로직 실행
	// if (!HasAuthority(&ActivationInfo))
	// {
	// 	return;
	// }
	//
	// // Indicator 클래스 유효성 체크
	// if (!IndicatorActorClass)
	// {
	// 	UE_LOG(LogTemp, Error, TEXT("UMSGA_Boss_Sevarog_Meteor: IndicatorActorClass is not set!"));
	// 	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	// 	return;
	// }
	//
	//  // 추적 + 스폰 태스크 시작
	// UMSAT_ChaseAndSpawnMeteor* ChaseTask = UMSAT_ChaseAndSpawnMeteor::CreateTask(
	// 	this,
	// 	ChaseDuration,
	// 	SpawnInterval,
	// 	IndicatorActorClass,
	// 	IndicatorParams,
	// 	MeteorDamageEffect);
	//
	// ChaseTask->OnChaseComplete.AddDynamic(this, &UMSGA_Boss_Sevarog_Meteor::OnChaseComplete);
	// ChaseTask->OnIndicatorSpawned.AddDynamic(this, &UMSGA_Boss_Sevarog_Meteor::OnIndicatorSpawned);
	// ChaseTask->ReadyForActivation();
}

void UMSGA_Boss_Sevarog_Meteor::CancelAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UMSGA_Boss_Sevarog_Meteor::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UMSGA_Boss_Sevarog_Meteor::OnCompleteCallback()
{
	// bool bReplicatedEndAbility = true;
	// bool bWasCancelled = false;
	// EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicatedEndAbility, bWasCancelled);
}

void UMSGA_Boss_Sevarog_Meteor::OnInterruptedCallback()
{
	bool bReplicatedEndAbility = true;
	bool bWasCancelled = true;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicatedEndAbility, bWasCancelled);
}

void UMSGA_Boss_Sevarog_Meteor::OnChaseComplete()
{
	bool bReplicatedEndAbility = true;
	bool bWasCancelled = false;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicatedEndAbility, bWasCancelled);
}

void UMSGA_Boss_Sevarog_Meteor::OnEventReceivedCallback(FGameplayTag EventTag, FGameplayEventData EventData)
{
	// 서버가 아니면 리턴
	if (GetCurrentActorInfo()->AvatarActor->GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	// Indicator 클래스 유효성 체크
	if (!IndicatorActorClass)
	{
		UE_LOG(LogTemp, Error, TEXT("UMSGA_Boss_Sevarog_Meteor: IndicatorActorClass is not set!"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// 추적 + 스폰 태스크 시작
	UMSAT_ChaseAndSpawnMeteor* ChaseTask = UMSAT_ChaseAndSpawnMeteor::CreateTask(
		this,
		ChaseDuration,
		SpawnInterval,
		IndicatorActorClass,
		IndicatorParams,
		MeteorDamageEffect);
	
	ChaseTask->OnChaseComplete.AddDynamic(this, &UMSGA_Boss_Sevarog_Meteor::OnChaseComplete);
	ChaseTask->OnIndicatorSpawned.AddDynamic(this, &UMSGA_Boss_Sevarog_Meteor::OnIndicatorSpawned);
	ChaseTask->ReadyForActivation();
}

void UMSGA_Boss_Sevarog_Meteor::OnIndicatorSpawned(AMSIndicatorActor* Indicator, FVector Location)
{
	if (!Indicator)
	{
		return;
	}

	// 추가 처리 (사운드, VFX 등)
	// Task에서 이미 데미지 정보를 설정하므로 여기서는 추가 이펙트만 처리
	
	// 예: GameplayCue 발동
	// UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	// if (SourceASC)
	// {
	//     FGameplayCueParameters CueParams;
	//     CueParams.Location = Location;
	//     SourceASC->ExecuteGameplayCue(MSGameplayTags::GameplayCue_Meteor_Warning, CueParams);
	// }
}
