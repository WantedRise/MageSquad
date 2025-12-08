// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AttributeSets/MS_PlayerAttributeSet.h"
#include "GameplayEffectExtension.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Net/UnrealNetwork.h"

UMS_PlayerAttributeSet::UMS_PlayerAttributeSet()
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

void UMS_PlayerAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
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

void UMS_PlayerAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 각 속성을 소유자에게만 복제. REPNOTIFY_Always는 값을 항상 OnRep 호출하도록 설정한다.
	DOREPLIFETIME_CONDITION_NOTIFY(UMS_PlayerAttributeSet, MaxHealth, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMS_PlayerAttributeSet, Health, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMS_PlayerAttributeSet, HealthRegen, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMS_PlayerAttributeSet, Defense, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMS_PlayerAttributeSet, DodgeRate, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMS_PlayerAttributeSet, MoveSpeedMod, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMS_PlayerAttributeSet, DamageMod, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMS_PlayerAttributeSet, SpellSizeMod, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMS_PlayerAttributeSet, CooldownReduction, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMS_PlayerAttributeSet, CritChance, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMS_PlayerAttributeSet, CritDamage, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMS_PlayerAttributeSet, PickupRangeMod, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMS_PlayerAttributeSet, ExperienceGainMod, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMS_PlayerAttributeSet, Luck, COND_OwnerOnly, REPNOTIFY_Always);
}

void UMS_PlayerAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMS_PlayerAttributeSet, Health, OldValue);
}

void UMS_PlayerAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMS_PlayerAttributeSet, MaxHealth, OldValue);
}

void UMS_PlayerAttributeSet::OnRep_HealthRegen(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMS_PlayerAttributeSet, HealthRegen, OldValue);
}

void UMS_PlayerAttributeSet::OnRep_Defense(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMS_PlayerAttributeSet, Defense, OldValue);
}

void UMS_PlayerAttributeSet::OnRep_DodgeRate(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMS_PlayerAttributeSet, DodgeRate, OldValue);
}

void UMS_PlayerAttributeSet::OnRep_MoveSpeedMod(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMS_PlayerAttributeSet, MoveSpeedMod, OldValue);
}

void UMS_PlayerAttributeSet::OnRep_DamageMod(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMS_PlayerAttributeSet, DamageMod, OldValue);
}

void UMS_PlayerAttributeSet::OnRep_SpellSizeMod(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMS_PlayerAttributeSet, SpellSizeMod, OldValue);
}

void UMS_PlayerAttributeSet::OnRep_CooldownReduction(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMS_PlayerAttributeSet, CooldownReduction, OldValue);
}

void UMS_PlayerAttributeSet::OnRep_CritChance(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMS_PlayerAttributeSet, CritChance, OldValue);
}

void UMS_PlayerAttributeSet::OnRep_CritDamage(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMS_PlayerAttributeSet, CritDamage, OldValue);
}

void UMS_PlayerAttributeSet::OnRep_PickupRangeMod(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMS_PlayerAttributeSet, PickupRangeMod, OldValue);
}

void UMS_PlayerAttributeSet::OnRep_ExperienceGainMod(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMS_PlayerAttributeSet, ExperienceGainMod, OldValue);
}

void UMS_PlayerAttributeSet::OnRep_Luck(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMS_PlayerAttributeSet, Luck, OldValue);
}
