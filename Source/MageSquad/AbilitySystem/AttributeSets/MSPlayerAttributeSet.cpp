// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AttributeSets/MSPlayerAttributeSet.h"
#include "GameplayEffectExtension.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Net/UnrealNetwork.h"

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

void UMSPlayerAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// 현재 체력 갱신 (최대 체력까지 Clamp)
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), UE_KINDA_SMALL_NUMBER, GetMaxHealth()));
	}

	// 최대 체력 갱신
	else if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		SetMaxHealth(GetMaxHealth());

		// 늘어난 체력만큼 회복
		const int32 AddedMaxHealth = GetMaxHealth() - GetHealth();
		SetHealth(FMath::Clamp(GetHealth() + AddedMaxHealth, UE_KINDA_SMALL_NUMBER, GetMaxHealth()));
	}

	// 체력 재생량 갱신
	else if (Data.EvaluatedData.Attribute == GetHealthRegenAttribute())
	{
		SetHealthRegen(GetHealthRegen());
	}

	// 방어력 갱신
	else if (Data.EvaluatedData.Attribute == GetDefenseAttribute())
	{
		SetDefense(GetDefense());
	}

	// 회피율 갱신
	else if (Data.EvaluatedData.Attribute == GetDodgeRateAttribute())
	{
		SetDodgeRate(GetDodgeRate());
	}

	// 이동 속도 보정 갱신
	else if (Data.EvaluatedData.Attribute == GetMoveSpeedModAttribute())
	{
		SetMoveSpeedMod(GetMoveSpeedMod());

		// 소유자의 무브먼트 컴포넌트 가져오기
		ACharacter* OwningCharacter = Cast<ACharacter>(GetOwningActor());
		UCharacterMovementComponent* CharacterMovement = OwningCharacter ? OwningCharacter->GetCharacterMovement() : nullptr;

		if (CharacterMovement)
		{
			if (DefaultMovementSpeed < 10.f)
			{
				DefaultMovementSpeed = CharacterMovement->MaxWalkSpeed;
			}

			// 소유자의 이동 속도 설정
			CharacterMovement->MaxWalkSpeed =
				DefaultMovementSpeed + (DefaultMovementSpeed * GetMoveSpeedMod());
		}
	}

	// 피해량 보정 갱신
	else if (Data.EvaluatedData.Attribute == GetDamageModAttribute())
	{
		SetDamageMod(GetDamageMod());
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
		SetPickupRangeMod(GetPickupRangeMod());
	}

	// 경험치 획득량 보정 갱신
	else if (Data.EvaluatedData.Attribute == GetExperienceGainModAttribute())
	{
		SetExperienceGainMod(GetExperienceGainMod());
	}

	// 행운 갱신
	else if (Data.EvaluatedData.Attribute == GetLuckAttribute())
	{
		SetLuck(GetLuck());
	}
}

void UMSPlayerAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 각 속성을 소유자에게만 복제. REPNOTIFY_Always는 값을 항상 OnRep 호출하도록 설정한다.
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
