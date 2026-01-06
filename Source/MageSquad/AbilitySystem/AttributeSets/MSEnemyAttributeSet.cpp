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
#include "GameFramework/CharacterMovementComponent.h"

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

#pragma region Health
	// Clamp Health to [0, MaxHealth]
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
			FGameplayEventData Payload;
			Payload.EventTag = MSGameplayTags::Shared_Event_DrawDamageNumber;
			Payload.EventMagnitude = DeltaHealth;

			// 치명타 포함 추가 태그 확인
			// EffectSpec에서 모든 태그를 가져와서 이벤트 데이터에 넘김
			FGameplayTagContainer SpecAssetTags;
			Data.EffectSpec.GetAllAssetTags(SpecAssetTags);
			Payload.InstigatorTags = SpecAssetTags;

			// EffectContext도 함께 전달(가해자/히트 결과 등 확장 가능)
			Payload.ContextHandle = Data.EffectSpec.GetEffectContext();

			// 받은 피해량 출력 이벤트 전달
			UAbilitySystemComponent* TargetASC = &Data.Target;
			TargetASC->HandleGameplayEvent(Payload.EventTag, &Payload);
		}

		SetCurrentHealth(FMath::Clamp(GetCurrentHealth(), 0.f, GetMaxHealth()));

		if (GetCurrentHealth() <= 0.f)
		{
			if (AMSBaseEnemy* OwnerEnemy = Cast<AMSBaseEnemy>(GetOwningActor()))
			{
				// HP소모 이벤트 호출
				FGameplayEventData Payload;
				Payload.EventTag = MSGameplayTags::Enemy_Event_HealthDepleted;
				UAbilitySystemComponent* TargetASC = &Data.Target;
				TargetASC->HandleGameplayEvent(Payload.EventTag, &Payload);
			}
		}
	}
#pragma endregion 
	
#pragma region MoveSpeed
	if (Data.EvaluatedData.Attribute == GetMoveSpeedAttribute())
	{
		// 받은 피해량 출력 이벤트 전달
		UAbilitySystemComponent* TargetASC = &Data.Target;
		if (AMSBaseEnemy* Target = Cast<AMSBaseEnemy>(TargetASC->GetOwnerActor()))
		{
			Target->GetCharacterMovement()->MaxWalkSpeed = GetMoveSpeed();
		}
	}
#pragma endregion
}
