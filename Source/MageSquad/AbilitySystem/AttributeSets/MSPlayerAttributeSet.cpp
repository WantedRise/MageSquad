// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AttributeSets/MSPlayerAttributeSet.h"
#include "GameplayEffectExtension.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interfaces/MSHitReactableInterface.h"

#include "Net/UnrealNetwork.h"

UMSPlayerAttributeSet::UMSPlayerAttributeSet()
{
	// �⺻ �Ӽ� �� ����. UI�� ���ذ� ���߱� ���� �ʱⰪ�� ����

	// ü�� �迭
	InitMaxHealth(100.0f);
	InitHealth(100.f);
	InitHealthRegen(0.f);

	// ���/ȸ�� �迭
	InitDefense(0.f);
	InitDodgeRate(0.f);

	// ����/�̵� �迭
	InitMoveSpeedMod(0.f);
	InitDamageMod(0.f);
	InitSpellSizeMod(0.f);
	InitCooldownReduction(0.f);

	// ġ��Ÿ �� ��Ÿ �迭
	InitCritChance(0.1f); // 10%
	InitCritDamage(1.5f); // 150%
	InitPickupRangeMod(0.f);
	InitExperienceGainMod(0.f);
	InitLuck(0.f);
}

void UMSPlayerAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// �ִ� ü�� ���� ��, ĳ�� ����
	if (Attribute == GetMaxHealthAttribute())
	{
		CachedOldMaxHealth = GetMaxHealth();
	}
}

void UMSPlayerAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// ���� ü�� ���� (�ִ� ü�±��� Clamp)
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), UE_KINDA_SMALL_NUMBER, GetMaxHealth()));
	}

	// �ִ� ü�� ����
	else if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		const float NewMaxHealth = GetMaxHealth();

		// �ִ� ü�� ���淮
		const float DeltaMaxHealth = NewMaxHealth - CachedOldMaxHealth;

		// �ִ� ü���� ���� ���
		if (DeltaMaxHealth > 0.f)
		{
			// �ִ� ü�� �����и�ŭ ���� ü�� ����
			SetHealth(GetHealth() + DeltaMaxHealth);
		}
		// �ִ� ü���� ������ ���
		else
		{
			// �ִ� ü�� ���Һи�ŭ ���� ü�� ����
			SetHealth(FMath::Min(GetHealth(), NewMaxHealth));
		}

		SetHealth(FMath::Clamp(GetHealth(), 0.f, NewMaxHealth));
	}
	// �̵� �ӵ� ���� ����
	else if (Data.EvaluatedData.Attribute == GetMoveSpeedModAttribute())
	{
		// �� ���� �� (ex. 0.1 = +10%)
		float NewMoveSpeedMod = GetMoveSpeedMod();

		// �ּ�/�ִ� �̵� �ӵ� Clamp (-90% ~ +300%)
		NewMoveSpeedMod = FMath::Clamp(NewMoveSpeedMod, -0.9f, 3.0f);
		SetMoveSpeedMod(NewMoveSpeedMod);

		// �ƹ�Ÿ�� �����Ʈ ������Ʈ ��������
		ACharacter* Avatar = Cast<ACharacter>(Data.Target.GetAvatarActor());
		if (!Avatar) return;

		UCharacterMovementComponent* MoveComp = Avatar->GetCharacterMovement();
		if (!MoveComp) return;

		// �⺻ �̵� �ӵ��� �� ���� ĳ��
		if (DefaultMovementSpeed <= 0.f)
		{
			DefaultMovementSpeed = MoveComp->MaxWalkSpeed;
		}

		// ���� �̵� �ӵ� = �⺻ �ӵ� * (1 + ���� ��)
		// ex) ���� 0.1 �� 1.1��, -0.3 �� 0.7��
		const float FinalMoveSpeed = DefaultMovementSpeed * (1.f + NewMoveSpeedMod);
		MoveComp->MaxWalkSpeed = FinalMoveSpeed;
	}
	// �ֹ� ũ�� ���� ����
	else if (Data.EvaluatedData.Attribute == GetSpellSizeModAttribute())
	{
		SetSpellSizeMod(GetSpellSizeMod());
	}

	// ��ٿ� ���� ����
	else if (Data.EvaluatedData.Attribute == GetCooldownReductionAttribute())
	{
		SetCooldownReduction(GetCooldownReduction());
	}

	// ġ��Ÿ Ȯ�� ����
	else if (Data.EvaluatedData.Attribute == GetCritChanceAttribute())
	{
		SetCritChance(GetCritChance());
	}

	// ġ��Ÿ ���� ����
	else if (Data.EvaluatedData.Attribute == GetCritDamageAttribute())
	{
		SetCritDamage(GetCritDamage());
	}

	// ȹ�� �ݰ� ���� ����
	else if (Data.EvaluatedData.Attribute == GetPickupRangeModAttribute())
	{
		// �� ȹ�� �ݰ�
		float NewRangeMod = GetMoveSpeedMod();
		SetPickupRangeMod(NewRangeMod);

		// Todo: ������ | ����ġ �ý��� ���� ��, ȹ�� �ݰ� �Ӽ� ���� �����ϱ�
	}
}

void UMSPlayerAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	/*
	* �� �Ӽ��� �����ڿ��Ը� ����
	* REPNOTIFY_Always�� ���� �׻� OnRep ȣ���ϵ��� ����
	* REPNOTIFY_Always�� �ε� �Ҽ��� �ݿø����� ���� ���� ������� �ʴ��� OnRep �Լ��� ȣ��ǵ��� �Ѵ�.
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
