// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GA/Enemy/MSGA_EnemyGroggy.h"

#include "AbilitySystemComponent.h"
#include "MSGameplayTags.h"
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

		UMSAT_PlayMontageAndWaitForEvent* AttackTask =
			UMSAT_PlayMontageAndWaitForEvent::CreateTask(this, GroggyMontage, Tags);

		AttackTask->OnCompleted.AddDynamic(this, &UMSGA_EnemyGroggy::OnCompleteCallback);
		AttackTask->OnInterrupted.AddDynamic(this, &UMSGA_EnemyGroggy::OnInterruptedCallback);
		AttackTask->OnEventReceived.AddDynamic(this, &UMSGA_EnemyGroggy::OnEventReceivedCallback);
		AttackTask->ReadyForActivation();
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
		// 람다를 사용해 다음 프레임에 실행되도록 예약
		GetWorld()->GetTimerManager().SetTimerForNextTick([this, EnemyAIController]()
		{
			if (!IsValid(this) || !Owner || !Owner->GetMesh())
			{
				return;
			}
			
			if (USkeletalMesh* NewMeshAsset = Owner->GetPhase2SkeletalMesh())
			{
				Owner->Multicast_TransitionToPhase2();
				
				if (HasAuthorityOrPredictionKey(CurrentActorInfo, &CurrentActivationInfo))
				{
					if (RecoveryEffectClass == nullptr)
					{
						UE_LOG(LogTemp, Warning, TEXT("RecoveryEffectClass is nullptr!"));
						return;
					}
				
					UAbilitySystemComponent* ASC = Owner->GetAbilitySystemComponent();
					// GameplayEffectSpec 생성
					FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
					Context.AddSourceObject(this);

					FGameplayEffectSpecHandle SpecHandle =
						ASC->MakeOutgoingSpec(RecoveryEffectClass, 1.f, Context);

					if (!SpecHandle.IsValid())
					{
						return;
					}
				
					// GameplayEffect 적용
					ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
					
					UE_LOG(LogTemp, Warning, TEXT("[%s] ApplyGameplayEffectSpecToSelf"),
						   HasAuthority(&CurrentActivationInfo) ? TEXT("Server") : TEXT("Client"));
					
					EnemyAIController->GetBlackboardComponent()->SetValueAsBool(EnemyAIController->GetIsGroggyKey(), false);
					Owner->GetAbilitySystemComponent()->AddLooseGameplayTag(MSGameplayTags::Enemy_State_Phase2);
					Owner->SetActorEnableCollision(true);
				}
			}

			else
			{
				UE_LOG(LogTemp, Error, TEXT("Phase2SkeletalMesh is NOT assigned in Blueprint!"));
			}
		});

		UE_LOG(LogTemp, Warning, TEXT("[%s] TargetMesh: %s"),
		       HasAuthority(&CurrentActivationInfo) ? TEXT("Server") : TEXT("Client"),
		       Owner->GetMesh() ? *Owner->GetMesh()->GetName() : TEXT("NULL"));
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
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
