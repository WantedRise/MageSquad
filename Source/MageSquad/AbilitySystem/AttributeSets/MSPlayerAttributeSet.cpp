// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AttributeSets/MSPlayerAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Player/MSPlayerCharacter.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Net/UnrealNetwork.h"

#include "MSGameplayTags.h"

UMSPlayerAttributeSet::UMSPlayerAttributeSet()
{
	// 기본 속성 값 설정. UI의 기준과 맞추기 위해 초기값을 지정

	// 체력 계열
	InitMaxHealth(100.0f);
	InitHealth(100.f);
	InitHealthRegen(0.f);

	// 방어/회피 계열
	InitDefense(0.f);
	InitDodgeRate(0.f);

	// 공격/이동 계열
	InitMoveSpeedMod(0.f);
	InitDamageMod(0.f);
	InitSpellSizeMod(0.f);
	InitCooldownReduction(0.f);

	// 치명타 및 기타 계열
	InitCritChance(0.1f); // 10%
	InitCritDamage(1.5f); // 150%
	InitPickupRangeMod(0.f);
	InitExperienceGainMod(0.f);
	InitLuck(0.f);
}

void UMSPlayerAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// 현재 체력 변경 시, 캐시 저장
	if (Attribute == GetHealthAttribute())
	{
		CachedOldHealth = GetHealth();
	}

	// 최대 체력 변경 시, 캐시 저장
	if (Attribute == GetMaxHealthAttribute())
	{
		CachedOldMaxHealth = GetMaxHealth();
	}
}

void UMSPlayerAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// 현재 체력 갱신 (최대 체력까지 Clamp)
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		UAbilitySystemComponent* TargetASC = &Data.Target;
		if (!TargetASC) return;

		// 초기화중이면 체력 변경에 따른 이벤트 로직 스킵
		if (TargetASC->HasMatchingGameplayTag(MSGameplayTags::Shared_State_Init))
		{
			return;
		}

		/*
		* 받은 피해량 출력 이벤트 전달 로직 (DamageFloater)
		*/
		{
			//// 현재 체력 및 받은 피해량 계산
			//const float NewHealth = GetHealth();
			//const float DeltaHealth = NewHealth - CachedOldHealth;

			//// 이벤트 데이터에 이벤트 태그 + 최종 피해량 저장
			//FGameplayEventData Payload;
			//Payload.EventTag = MSGameplayTags::Shared_Event_DrawDamageNumber;
			//Payload.EventMagnitude = DeltaHealth;

			//// 치명타 포함 추가 태그 확인
			//// EffectSpec에서 모든 태그를 가져와서 이벤트 데이터에 넘김
			//FGameplayTagContainer SpecAssetTags;
			//Data.EffectSpec.GetAllAssetTags(SpecAssetTags);
			//Payload.InstigatorTags = SpecAssetTags;

			//// EffectContext도 함께 전달(가해자/히트 결과 등 확장 가능)
			//Payload.ContextHandle = Data.EffectSpec.GetEffectContext();

			//// 받은 피해량 출력 이벤트 전달
			//TargetASC->HandleGameplayEvent(Payload.EventTag, &Payload);
		}

		/*
		* 카메라 흔들림 수행 명령 로직
		*/
		{
			// ASC와 AttributeSet은 PlayerState에서 가지고 있음.
			// 따라서 ASC로 한 번 거쳐서 로컬 플레이어를 가져와야 함
			AMSPlayerCharacter* MSOnwer = Cast<AMSPlayerCharacter>(TargetASC->GetAvatarActor());
			if (MSOnwer && MSOnwer->HasAuthority())
			{
				// 카메라 흔들림 강도 (경우에 따라 설정하기)
				const float ShakeScale = 0.3f;

				// 체력이 변경된 로컬 클라이언트에게 카메라 흔들림 수행하라고 명령
				MSOnwer->ClientRPCPlayHealthShake(ShakeScale);
			}
		}

		/*
		* 부활/관전 로직
		*/
		{
			const float NewHealth = GetHealth();
			if (NewHealth <= 0.f)
			{
				// 서버에서만 사망 처리
				if (AMSPlayerCharacter* OwnerChar = Cast<AMSPlayerCharacter>(TargetASC->GetAvatarActor()))
				{
					// 캐릭터의 사망 처리 함수 호출
					// Test를 위한 주석 처리 - 임희섭
					// OwnerChar->SetCharacterOnDead();
				}
			}
		}

		SetHealth(FMath::Clamp(GetHealth(), UE_KINDA_SMALL_NUMBER, GetMaxHealth()));
	}
	// 최대 체력 갱신
	else if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		const float NewMaxHealth = GetMaxHealth();

		// 최대 체력 변경량
		const float DeltaMaxHealth = NewMaxHealth - CachedOldMaxHealth;

		// 최대 체력이 오른 경우
		if (DeltaMaxHealth > 0.f)
		{
			// 최대 체력 증가분만큼 현재 체력 설정
			SetHealth(GetHealth() + DeltaMaxHealth);
		}
		// 최대 체력이 감소한 경우
		else
		{
			// 최대 체력 감소분만큼 현재 체력 설정
			SetHealth(FMath::Min(GetHealth(), NewMaxHealth));
		}

		SetHealth(FMath::Clamp(GetHealth(), 0.f, NewMaxHealth));
	}
	// 이동 속도 보정 갱신
	else if (Data.EvaluatedData.Attribute == GetMoveSpeedModAttribute())
	{
		// 새 보정 값 (ex. 0.1 = +10%)
		float NewMoveSpeedMod = GetMoveSpeedMod();

		// 최소/최대 이동 속도 Clamp (-90% ~ +300%)
		NewMoveSpeedMod = FMath::Clamp(NewMoveSpeedMod, -0.9f, 3.0f);
		SetMoveSpeedMod(NewMoveSpeedMod);

		// 아바타의 무브먼트 컴포넌트 가져오기
		ACharacter* Avatar = Cast<ACharacter>(Data.Target.GetAvatarActor());
		if (!Avatar) return;

		UCharacterMovementComponent* MoveComp = Avatar->GetCharacterMovement();
		if (!MoveComp) return;

		// 기본 이동 속도를 한 번만 캐싱
		if (DefaultMovementSpeed <= 0.f)
		{
			DefaultMovementSpeed = MoveComp->MaxWalkSpeed;
		}

		// 최종 이동 속도 = 기본 속도 * (1 + 보정 값)
		// ex) 보정 0.1 → 1.1배, -0.3 → 0.7배
		const float FinalMoveSpeed = DefaultMovementSpeed * (1.f + NewMoveSpeedMod);
		MoveComp->MaxWalkSpeed = FinalMoveSpeed;
	}
	// 주문 크기 보정 갱신
	else if (Data.EvaluatedData.Attribute == GetSpellSizeModAttribute())
	{
		SetSpellSizeMod(GetSpellSizeMod());
	}

	// 쿨다운 감소 갱신
	else if (Data.EvaluatedData.Attribute == GetCooldownReductionAttribute())
	{
		SetCooldownReduction(GetCooldownReduction());
	}

	// 치명타 확률 갱신
	else if (Data.EvaluatedData.Attribute == GetCritChanceAttribute())
	{
		SetCritChance(GetCritChance());
	}

	// 치명타 피해 갱신
	else if (Data.EvaluatedData.Attribute == GetCritDamageAttribute())
	{
		SetCritDamage(GetCritDamage());
	}

	// 획득 반경 보정 갱신
	else if (Data.EvaluatedData.Attribute == GetPickupRangeModAttribute())
	{
		// 새 획득 반경
		float NewRangeMod = GetMoveSpeedMod();
		SetPickupRangeMod(NewRangeMod);
	}
}

void UMSPlayerAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	/*
	* 각 속성을 소유자에게만 복제
	* REPNOTIFY_Always는 값을 항상 OnRep 호출하도록 설정
	* REPNOTIFY_Always는 부동 소수점 반올림으로 인해 값이 변경되지 않더라도 OnRep 함수가 호출되도록 한다.
	*/
	DOREPLIFETIME_CONDITION_NOTIFY(UMSPlayerAttributeSet, MaxHealth, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMSPlayerAttributeSet, Health, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMSPlayerAttributeSet, HealthRegen, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMSPlayerAttributeSet, Defense, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMSPlayerAttributeSet, DodgeRate, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMSPlayerAttributeSet, MoveSpeedMod, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMSPlayerAttributeSet, DamageMod, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMSPlayerAttributeSet, SpellSizeMod, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMSPlayerAttributeSet, CooldownReduction, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMSPlayerAttributeSet, CritChance, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMSPlayerAttributeSet, CritDamage, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMSPlayerAttributeSet, PickupRangeMod, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMSPlayerAttributeSet, ExperienceGainMod, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMSPlayerAttributeSet, Luck, COND_OwnerOnly, REPNOTIFY_Always);
}

void UMSPlayerAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMSPlayerAttributeSet, Health, OldValue);
}

void UMSPlayerAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMSPlayerAttributeSet, MaxHealth, OldValue);
}

void UMSPlayerAttributeSet::OnRep_HealthRegen(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMSPlayerAttributeSet, HealthRegen, OldValue);
}

void UMSPlayerAttributeSet::OnRep_Defense(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMSPlayerAttributeSet, Defense, OldValue);
}

void UMSPlayerAttributeSet::OnRep_DodgeRate(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMSPlayerAttributeSet, DodgeRate, OldValue);
}

void UMSPlayerAttributeSet::OnRep_MoveSpeedMod(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMSPlayerAttributeSet, MoveSpeedMod, OldValue);
}

void UMSPlayerAttributeSet::OnRep_DamageMod(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMSPlayerAttributeSet, DamageMod, OldValue);
}

void UMSPlayerAttributeSet::OnRep_SpellSizeMod(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMSPlayerAttributeSet, SpellSizeMod, OldValue);
}

void UMSPlayerAttributeSet::OnRep_CooldownReduction(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMSPlayerAttributeSet, CooldownReduction, OldValue);
}

void UMSPlayerAttributeSet::OnRep_CritChance(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMSPlayerAttributeSet, CritChance, OldValue);
}

void UMSPlayerAttributeSet::OnRep_CritDamage(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMSPlayerAttributeSet, CritDamage, OldValue);
}

void UMSPlayerAttributeSet::OnRep_PickupRangeMod(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMSPlayerAttributeSet, PickupRangeMod, OldValue);
}

void UMSPlayerAttributeSet::OnRep_ExperienceGainMod(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMSPlayerAttributeSet, ExperienceGainMod, OldValue);
}

void UMSPlayerAttributeSet::OnRep_Luck(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMSPlayerAttributeSet, Luck, OldValue);
}