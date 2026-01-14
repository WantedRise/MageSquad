// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/AttributeSets/MSEnemyAttributeSet.h"

#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Enemy/MSBaseEnemy.h"
#include "Enemy/AIController/MSBaseAIController.h"
#include "Interfaces/MSHitReactableInterface.h"

#include "MSGameplayTags.h"
#include "Player/MSPlayerController.h"
#include "Enemy/MSBossEnemy.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EngineUtils.h"

UMSEnemyAttributeSet::UMSEnemyAttributeSet()
{
}

void UMSEnemyAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UMSEnemyAttributeSet, CurrentHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMSEnemyAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMSEnemyAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMSEnemyAttributeSet, AttackDamage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMSEnemyAttributeSet, AttackRange, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMSEnemyAttributeSet, DropExpValue, COND_None, REPNOTIFY_Always);
}

void UMSEnemyAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// 현재 체력 변경 시, 캐시 저장
	if (Attribute == GetCurrentHealthAttribute())
	{
		CachedOldCurrentHealth = GetCurrentHealth();
	}
}

void UMSEnemyAttributeSet::OnRep_CurrentHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMSEnemyAttributeSet, CurrentHealth, OldValue);
}

void UMSEnemyAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMSEnemyAttributeSet, MaxHealth, OldValue);
}

void UMSEnemyAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMSEnemyAttributeSet, MoveSpeed, OldValue);
}

void UMSEnemyAttributeSet::OnRep_AttackDamage(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMSEnemyAttributeSet, AttackDamage, OldValue);
}

void UMSEnemyAttributeSet::OnRep_AttackRange(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMSEnemyAttributeSet, AttackRange, OldValue);
}

void UMSEnemyAttributeSet::OnRep_DropExpValue(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMSEnemyAttributeSet, DropExpValue, OldValue);
}

void UMSEnemyAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.Target.GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	if (Data.EvaluatedData.Attribute == GetCurrentHealthAttribute())
	{
		/*
		* 김준형
		* 받은 피해량 출력 이벤트 전달 로직 구현
		*/
		{
			// 현재 체력 및 받은 피해량 계산
			const float NewHealth = GetCurrentHealth();
			const float DeltaHealth = NewHealth - CachedOldCurrentHealth;

			// 이벤트 데이터에 이벤트 태그 + 최종 피해량 저장
			UAbilitySystemComponent* TargetASC = &Data.Target;
			if (!FMath::IsNearlyZero(DeltaHealth))
			{
				// 치명타 여부 태그 저장
				FGameplayTagContainer SpecAssetTags;
				Data.EffectSpec.GetAllAssetTags(SpecAssetTags);
				const bool bIsCritical = SpecAssetTags.HasTag(MSGameplayTags::Hit_Critical);

				// 받은 피해량 누적
				QueueDamageFloater(DeltaHealth, bIsCritical);
			}

			SetCurrentHealth(FMath::Clamp(GetCurrentHealth(), 0.f, GetMaxHealth()));

			if (GetCurrentHealth() <= 0.f)
			{
				if (AMSBaseEnemy* OwnerEnemy = Cast<AMSBaseEnemy>(GetOwningActor()))
				{
					// HP소모 이벤트 호출
					FGameplayEventData HandleHealthPayload;
					HandleHealthPayload.EventTag = MSGameplayTags::Enemy_Event_HealthDepleted;
					TargetASC->HandleGameplayEvent(HandleHealthPayload.EventTag, &HandleHealthPayload);
				}
			}
		}
	}

	if (Data.EvaluatedData.Attribute == GetMoveSpeedAttribute())
	{
		UAbilitySystemComponent* TargetASC = &Data.Target;
		if (AMSBaseEnemy* Target = Cast<AMSBaseEnemy>(TargetASC->GetOwnerActor()))
		{
			Target->GetCharacterMovement()->MaxWalkSpeed = GetMoveSpeed();
		}
	}
}


void UMSEnemyAttributeSet::QueueDamageFloater(float DeltaHealth, bool bIsCritical)
{
	// 받은 피해량 누적 + 치명타 여부 OR 계산
	PendingDamageFloater += DeltaHealth;
	bPendingCritical |= bIsCritical;

	AActor* OwnerActor = GetOwningActor();
	UWorld* World = OwnerActor ? OwnerActor->GetWorld() : nullptr;
	if (!World)
	{
		FlushDamageFloater();
		return;
	}

	// 피해량 전송 주기마다 피해량 전송
	FTimerManager& TimerManager = World->GetTimerManager();
	if (!TimerManager.IsTimerActive(DamageFloaterFlushHandle))
	{
		TimerManager.SetTimer(DamageFloaterFlushHandle, this, &UMSEnemyAttributeSet::FlushDamageFloater, DamageFloaterBatchInterval, false);
	}
}

void UMSEnemyAttributeSet::FlushDamageFloater()
{
	if (FMath::IsNearlyZero(PendingDamageFloater))
	{
		PendingDamageFloater = 0.f;
		bPendingCritical = false;
		return;
	}

	// 대미지 플로터는 주변 플레이어에게만 전송
	AActor* OwnerActor = GetOwningActor();
	UWorld* World = OwnerActor ? OwnerActor->GetWorld() : nullptr;
	if (!World || !OwnerActor)
	{
		PendingDamageFloater = 0.f;
		bPendingCritical = false;
		return;
	}

	const FVector SourceLocation = OwnerActor->GetActorLocation();
	const float RangeSq = DamageFloaterRange * DamageFloaterRange;

	// 월드 내 PlayerContoller 탐색
	for (APlayerController* OtherPC : TActorRange<APlayerController>(GetWorld()))
	{
		AMSPlayerController* PC = Cast<AMSPlayerController>(OtherPC);
		if (!PC) continue;

		APawn* Pawn = PC->GetPawn();
		if (!Pawn) continue;

		// 거리 기반 필터로 네트워크 전송을 줄임
		if (FVector::DistSquared(Pawn->GetActorLocation(), SourceLocation) > RangeSq)
		{
			continue;
		}

		PC->ClientRPCShowDamageFloater(PendingDamageFloater, bPendingCritical, SourceLocation);
	}

	PendingDamageFloater = 0.f;
	bPendingCritical = false;
}
