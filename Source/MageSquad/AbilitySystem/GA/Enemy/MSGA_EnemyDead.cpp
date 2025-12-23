// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GA/Enemy/MSGA_EnemyDead.h"

#include "AbilitySystemComponent.h"
#include "MSGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/AttributeSets/MSEnemyAttributeSet.h"
#include "Actors/Experience/MSExperienceOrb.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Enemy/MSBaseEnemy.h"
#include "Enemy/AIController/MSBaseAIController.h"

UMSGA_EnemyDead::UMSGA_EnemyDead()
{
	// 어빌리티 식별 Tag
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(MSGameplayTags::Enemy_Ability_Dead);
	SetAssetTags(TagContainer);

	// 활성화 시 Owner에게 부여되는 Tag
	ActivationOwnedTags.AddTag(MSGameplayTags::Enemy_State_Dead);
	
	BlockAbilitiesWithTag.AddTag(MSGameplayTags::Enemy_Ability_Dead)	;
}

void UMSGA_EnemyDead::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (UAnimMontage* DeadMontage = Owner->GetDeadMontage())
	{
		UAbilityTask_PlayMontageAndWait* EnemyDeadTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("Dead"), DeadMontage);
		EnemyDeadTask->OnCompleted.AddDynamic(this, &UMSGA_EnemyDead::OnCompleteCallback); // 몽타주가 끝나면 호출될 함수
		EnemyDeadTask->OnInterrupted.AddDynamic(this, &UMSGA_EnemyDead::OnInterruptedCallback); // 몽타주가 중단되면 호출될 함수
		//EnemyDeadTask->OnBlendOut.AddDynamic(this, &UMSGA_EnemyDead::OnCompleteCallback); // 추가
		EnemyDeadTask->ReadyForActivation();
		UE_LOG(LogTemp, Warning, TEXT("[%s] Enemy Dead Ability Being"), *GetAvatarActorFromActorInfo()->GetName())
		
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
	
	UE_LOG(LogTemp, Warning, TEXT("[%s] Enemy Dead Ability End"), *GetAvatarActorFromActorInfo()->GetName())
	
	if (AMSBaseAIController* EnemyAIController = Cast<AMSBaseAIController>(Owner->GetController()))
	{
		EnemyAIController->GetBlackboardComponent()->SetValueAsBool(EnemyAIController->GetIsDeadKey(), false);
		EnemyAIController->GetBlackboardComponent()->SetValueAsBool(EnemyAIController->GetCanAttackKey(), false);
	}
	
	if (GetCurrentActorInfo()->AvatarActor->GetLocalRole() != ROLE_Authority)
	{
		return;
	}
	
	if (!ExpReward)
	{
		return;
	}
	
	AMSExperienceOrb* ExpObject = Cast<AMSExperienceOrb>(GetWorld()->SpawnActor<AMSExperienceOrb>(
		ExpReward, 
		Owner->GetActorLocation(), 
		FRotator(0.0f, 0.0f, 0.0f)));
	
	if (ExpObject)
	{
		// 3. ASC 확인
		if (UAbilitySystemComponent* ASC = Owner->GetAbilitySystemComponent())
		{
			// 4. AttributeSet 확인 및 캐스팅
			if (const UMSEnemyAttributeSet* EnemyAS = Cast<UMSEnemyAttributeSet>(ASC->GetAttributeSet(UMSEnemyAttributeSet::StaticClass())))
			{
				// 모든 값이 안전할 때만 값 대입
				ExpObject->ExperienceValue = EnemyAS->GetDropExpValue();
			}
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
