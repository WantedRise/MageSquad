// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GA/Enemy/MSGA_EnemyDead.h"

#include "AbilitySystemComponent.h"
#include "MSGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/AttributeSets/MSEnemyAttributeSet.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Enemy/MSBaseEnemy.h"
#include "Enemy/AIController/MSBaseAIController.h"

#include "Actors/Items/MSExperienceOrb.h"
#include "Actors/Items/MSMagnetOrb.h"
#include "Actors/Items/MSPotionOrb.h"

UMSGA_EnemyDead::UMSGA_EnemyDead()
{
	// 어빌리티 식별 Tag
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(MSGameplayTags::Enemy_Ability_Dead);
	SetAssetTags(TagContainer);

	// 활성화 시 Owner에게 부여되는 Tag
	ActivationOwnedTags.AddTag(MSGameplayTags::Enemy_State_Dead);

	// BlockAbilitiesWithTag.AddTag(MSGameplayTags::Enemy_Ability_Dead);
}

void UMSGA_EnemyDead::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 1. ASC 가져오기
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		// 2. 파라미터 설정
		FGameplayCueParameters CueParams;
		CueParams.Instigator = GetAvatarActorFromActorInfo();
		CueParams.TargetAttachComponent = Owner->GetMesh();

		// 3. ASC를 통해 GameplayCue 실행
		FGameplayTag CueTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Dissolve"));
		ASC->ExecuteGameplayCue(CueTag, CueParams);
	}

	if (UAnimMontage* DeadMontage = Owner->GetDeadMontage())
	{
		UAbilityTask_PlayMontageAndWait* EnemyDeadTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("Dead"), DeadMontage);
		EnemyDeadTask->OnCompleted.AddDynamic(this, &UMSGA_EnemyDead::OnCompleteCallback); // 몽타주가 끝나면 호출될 함수
		EnemyDeadTask->OnInterrupted.AddDynamic(this, &UMSGA_EnemyDead::OnInterruptedCallback); // 몽타주가 중단되면 호출될 함수
		EnemyDeadTask->ReadyForActivation();
		//UE_LOG(LogTemp, Warning, TEXT("[%s] Enemy Dead Ability Being"), *GetAvatarActorFromActorInfo()->GetName())

		Owner->SetActorEnableCollision(false);
	}
}

void UMSGA_EnemyDead::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UMSGA_EnemyDead::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (AMSBaseAIController* EnemyAIController = Cast<AMSBaseAIController>(Owner->GetController()))
	{
		EnemyAIController->GetBlackboardComponent()->SetValueAsBool(EnemyAIController->GetIsDeadKey(), false);
		EnemyAIController->GetBlackboardComponent()->SetValueAsBool(EnemyAIController->GetCanAttackKey(), false);
	}

	if (GetCurrentActorInfo()->AvatarActor->GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	if (!ExpReward || !MagnetReward || !PotionReward)
	{
		return;
	}

	// 김준형 수정: 경험치 오브 드롭 -> 확률에 따라 아이템 오브(경험치, 자석, 포션) 랜덤 드롭
	// 아이템 오브 드롭 확률 구하기
	const float Roll = FMath::FRand(); // (0.0, 1.0)

	// 스폰할 클래스 선택
	TSubclassOf<AActor> SpawnClass = nullptr;

	// 90% 확률로 경험치 오브 드롭
	if (Roll < 0.90f)
	{
		SpawnClass = ExpReward;
	}
	// 8% 확률로 포션 오브 드롭
	else if (Roll < 0.98f)
	{
		SpawnClass = PotionReward;
	}
	// 2% 확률로 자석 오브 드롭
	else
	{
		SpawnClass = MagnetReward;
	}

	if (!SpawnClass) return;

	// 아이템 오브 스폰
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(
		SpawnClass,
		Owner->GetActorLocation(),
		FRotator::ZeroRotator,
		Params
	);

	if (!SpawnedActor) return;

	// 경험치 오브가 드롭된 경우에만 경험치량 보정 로직 수행
	AMSExperienceOrb* ExpObject = Cast<AMSExperienceOrb>(SpawnedActor);
	if (!ExpObject) return;

	// ASC 확인
	if (UAbilitySystemComponent* ASC = Owner->GetAbilitySystemComponent())
	{
		// AttributeSet 확인 및 캐스팅
		if (const UMSEnemyAttributeSet* EnemyAS = Cast<UMSEnemyAttributeSet>(
			ASC->GetAttributeSet(UMSEnemyAttributeSet::StaticClass())))
		{
			// 모든 값이 안전할 때만 값 대입
			ExpObject->ExperienceValue = EnemyAS->GetDropExpValue();
		}
	}
}

void UMSGA_EnemyDead::OnCompleteCallback()
{
	bool bReplicatedEndAbility = true;
	bool bWasCancelled = false;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicatedEndAbility, bWasCancelled);
}

void UMSGA_EnemyDead::OnInterruptedCallback()
{
	bool bReplicatedEndAbility = true;
	bool bWasCancelled = true;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicatedEndAbility, bWasCancelled);
}
