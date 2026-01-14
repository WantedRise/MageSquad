// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GA/Enemy/MSGA_EnemyGroggy.h"

#include "AbilitySystemComponent.h"
#include "MSGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/Tasks/MSAT_PlayMontageAndWaitForEvent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "Enemy/MSBossEnemy.h"
#include "AbilitySystem/AttributeSets/MSEnemyAttributeSet.h"
#include "Enemy/AIController/MSBossAIController.h"
#include "GameFramework/CharacterMovementComponent.h"


UMSGA_EnemyGroggy::UMSGA_EnemyGroggy()
{
	// 어빌리티 식별 Tag
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(MSGameplayTags::Enemy_Ability_Groggy);
	SetAssetTags(TagContainer);

	// 활성화 시 Owner에게 부여되는 Tag
	ActivationOwnedTags.AddTag(MSGameplayTags::Enemy_State_Groggy);

	ActivationBlockedTags.AddTag(MSGameplayTags::Enemy_Ability_Groggy);
	BlockAbilitiesWithTag.AddTag(MSGameplayTags::Enemy_Ability_Dead);
}

void UMSGA_EnemyGroggy::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                        const FGameplayAbilityActorInfo* ActorInfo,
                                        const FGameplayAbilityActivationInfo ActivationInfo,
                                        const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (ACharacter* CharacterOwner = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		if (UCharacterMovementComponent* CMC = CharacterOwner->GetCharacterMovement())
		{
			CMC->StopMovementImmediately();
			CMC->Velocity = FVector::ZeroVector;
			CMC->DisableMovement();
		}
	}

	GroggyCountAcc = 0;

	if (UAnimMontage* GroggyMontage = Owner->GetGroggyMontage())
	{
		FGameplayTagContainer Tags;
		Tags.AddTag(FGameplayTag::RequestGameplayTag("Enemy.Event.Groggy"));

		// UMSAT_PlayMontageAndWaitForEvent* GroggyTask =
		// 	UMSAT_PlayMontageAndWaitForEvent::CreateTask(this, GroggyMontage, Tags);
		//
		// GroggyTask->OnCompleted.AddDynamic(this, &UMSGA_EnemyGroggy::OnCompleteCallback);
		// GroggyTask->OnInterrupted.AddDynamic(this, &UMSGA_EnemyGroggy::OnInterruptedCallback);
		// GroggyTask->OnEventReceived.AddDynamic(this, &UMSGA_EnemyGroggy::OnEventReceivedCallback);
		// GroggyTask->ReadyForActivation();
		
		UAbilityTask_PlayMontageAndWait* EnemyDeadTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("Groggy"), GroggyMontage);
		EnemyDeadTask->OnCompleted.AddDynamic(this, &UMSGA_EnemyGroggy::OnCompleteCallback);
		EnemyDeadTask->OnBlendOut.AddDynamic(this, &UMSGA_EnemyGroggy::OnCompleteCallback);
		EnemyDeadTask->OnInterrupted.AddDynamic(this, &UMSGA_EnemyGroggy::OnInterruptedCallback);
		EnemyDeadTask->ReadyForActivation();
	}

	Owner->SetActorEnableCollision(false);
}

void UMSGA_EnemyGroggy::CancelAbility(const FGameplayAbilitySpecHandle Handle,
                                      const FGameplayAbilityActorInfo* ActorInfo,
                                      const FGameplayAbilityActivationInfo ActivationInfo,
                                      bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);

	bool bReplicatedEndAbility = true;
	bool bWasCancelled = true;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicatedEndAbility, bWasCancelled);
}

void UMSGA_EnemyGroggy::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                   const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
                                   bool bWasCancelled)
{
	// UE_LOG(LogTemp, Warning, TEXT("[Groggy] Boss EndAbility Called - bWasCancelled: %s, IsActive: %s"),
	// bWasCancelled ? TEXT("true") : TEXT("false"),
	// IsActive() ? TEXT("true") : TEXT("false"));
	//
	// // 이미 종료된 상태면 early return
	// if (!IsActive())
	// {
	// 	return;
	// }
	
	if (ACharacter* CharacterOwner = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		if (UCharacterMovementComponent* CMC = CharacterOwner->GetCharacterMovement())
		{
			CMC->SetMovementMode(MOVE_Walking);

			CMC->StopMovementImmediately();
			CMC->Velocity = FVector::ZeroVector;
		}
	}

	if (AMSBossAIController* EnemyAIController = Cast<AMSBossAIController>(Owner->GetController()))
	{
		// 필요한 정보를 미리 캡처
		TWeakObjectPtr<UGameplayAbility> WeakThis = this;
		TWeakObjectPtr<AMSBossEnemy> WeakOwner = Owner;
		TWeakObjectPtr<AMSBossAIController> WeakAIController = EnemyAIController;

		// Activation 정보는 값으로 복사 (구조체이므로)
		FGameplayAbilityActivationInfo CachedActivationInfo = CurrentActivationInfo;
		FGameplayAbilityActorInfo CachedActorInfo = *CurrentActorInfo;

		// RecoveryEffectClass도 미리 캡처 (TSubclassOf는 안전)
		TSubclassOf<UGameplayEffect> CachedRecoveryEffectClass = RecoveryEffectClass;

		// 람다를 사용해 다음 프레임에 실행되도록 예약
		GetWorld()->GetTimerManager().SetTimerForNextTick([WeakThis, WeakOwner, WeakAIController,
				CachedActivationInfo, CachedActorInfo, CachedRecoveryEffectClass]()
			{
				// 모든 약한 참조 유효성 검사
				if (!WeakThis.IsValid() || !WeakOwner.IsValid() || !WeakAIController.IsValid())
				{
					return;
				}

				AMSBossEnemy* LambdaOwner = WeakOwner.Get();
				AMSBossAIController* AIController = WeakAIController.Get();
				UGameplayAbility* Ability = WeakThis.Get();

				if (!LambdaOwner->GetMesh())
				{
					return;
				}
				USkeletalMesh* NewMeshAsset = LambdaOwner->GetPhase2SkeletalMesh();
				if (!NewMeshAsset)
				{
					UE_LOG(LogTemp, Error, TEXT("Phase2SkeletalMesh is NOT assigned in Blueprint!"));
					return;
				}
				LambdaOwner->Multicast_TransitionToPhase2();

				// Authority 체크 (캡처된 정보 사용)
				const bool bHasAuthority = CachedActorInfo.IsNetAuthority();
				if (!bHasAuthority)
				{
					return;
				}

				if (!CachedRecoveryEffectClass)
				{
					UE_LOG(LogTemp, Warning, TEXT("RecoveryEffectClass is nullptr!"));
					return;
				}

				UAbilitySystemComponent* ASC = LambdaOwner->GetAbilitySystemComponent();
				if (!ASC)
				{
					UE_LOG(LogTemp, Warning, TEXT("ASC is nullptr!"));
					return;
				}

				// GameplayEffectSpec 생성 및 적용
				FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
				Context.AddSourceObject(Ability);

				FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
					CachedRecoveryEffectClass, 1.f, Context);

				if (!SpecHandle.IsValid())
				{
					UE_LOG(LogTemp, Warning, TEXT("Failed to create EffectSpec!"));
					return;
				}

				ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);

				UE_LOG(LogTemp, Log, TEXT("[Server] ApplyGameplayEffectSpecToSelf succeeded"));

				// Blackboard 및 태그 업데이트
				if (UBlackboardComponent* BB = AIController->GetBlackboardComponent())
				{
					BB->SetValueAsBool(AIController->GetIsGroggyKey(), false);
				}

				ASC->AddLooseGameplayTag(MSGameplayTags::Enemy_State_Phase2);
				LambdaOwner->SetActorEnableCollision(true);
			});
		UE_LOG(LogTemp, Warning, TEXT("[%s] Enemy TargetMesh: %s"),
		       HasAuthority(&CurrentActivationInfo) ? TEXT("Server") : TEXT("Client"),
		       Owner->GetMesh() ? *Owner->GetMesh()->GetName() : TEXT("NULL"));
	}
	
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(GroggyTimerHandle);
	}

	UE_LOG(LogTemp, Warning, TEXT("[Groggy] Before Super::EndAbility"));
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	UE_LOG(LogTemp, Warning, TEXT("[Groggy] After Super::EndAbility"));

	if (UAbilitySystemComponent* ASC = Owner->GetAbilitySystemComponent())
	{
		while (ASC->HasMatchingGameplayTag(MSGameplayTags::Enemy_State_Groggy))
		{
			ASC->RemoveLooseGameplayTag(MSGameplayTags::Enemy_State_Groggy);
			UE_LOG(LogTemp, Warning, TEXT("[Groggy] Manually removed Groggy tag"));
		}
		
		UE_LOG(LogTemp, Warning, TEXT("[Groggy] Manually removed Groggy tag Complete"));
	}
}

void UMSGA_EnemyGroggy::OnCompleteCallback()
{
	bool bReplicatedEndAbility = true;
	bool bWasCancelled = false;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicatedEndAbility, bWasCancelled);
}

void UMSGA_EnemyGroggy::OnInterruptedCallback()
{
	bool bReplicatedEndAbility = true;
	bool bWasCancelled = true;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicatedEndAbility, bWasCancelled);
}

void UMSGA_EnemyGroggy::OnEventReceivedCallback(FGameplayTag EventTag, FGameplayEventData EventData)
{
	if (GroggyCountAcc >= 2)
	{
		UAnimInstance* AnimInstance = Owner->GetMesh()->GetAnimInstance();
		AnimInstance->Montage_JumpToSection(FName(TEXT("End")), AnimInstance->GetCurrentActiveMontage());
	}

	else
	{
		++GroggyCountAcc;
	}
}
