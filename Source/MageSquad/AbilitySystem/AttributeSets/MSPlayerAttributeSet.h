// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "MSPlayerAttributeSet.generated.h"

// Getter와 Setter를 자동 생성
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 작성자: 김준형
 * 작성일: 25/12/08
 * 
 * 플레이어 캐릭터의 AttributeSet 클래스
 * 플레이어 캐릭터의 모든 능력치 집합, 능력치 변경을 트래킹하고 복제함
 */
UCLASS()
class MAGESQUAD_API UMSPlayerAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UMSPlayerAttributeSet();

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	/*
	* 현재 체력
	* 캐릭터가 받는 피해에 따라 감소
	*/
	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UMSPlayerAttributeSet, Health)

	/*
	* 최대 체력
	*/
	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UMSPlayerAttributeSet, MaxHealth)

	/*
	* 체력 재생량
	* 초당 회복되는 체력량
	*/
	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing = OnRep_HealthRegen)
	FGameplayAttributeData HealthRegen;
	ATTRIBUTE_ACCESSORS(UMSPlayerAttributeSet, HealthRegen)

	/*
	* 방어력
	* 받는 피해를 감소시키는 계수
	*/
	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing = OnRep_Defense)
	FGameplayAttributeData Defense;
	ATTRIBUTE_ACCESSORS(UMSPlayerAttributeSet, Defense)

	/*
	* 회피율
	* 적의 공격을 회피할 확률(0 ~ 1)
	*/
	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing = OnRep_DodgeRate)
	FGameplayAttributeData DodgeRate;
	ATTRIBUTE_ACCESSORS(UMSPlayerAttributeSet, DodgeRate)

	/*
	* 이동 속도 보정
	* 기본 이동 속도에 대한 추가/감소 비율(ex. 0.1 = +10%)
	*/
	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing = OnRep_MoveSpeedMod)
	FGameplayAttributeData MoveSpeedMod;
	ATTRIBUTE_ACCESSORS(UMSPlayerAttributeSet, MoveSpeedMod)

	/*
	* 피해량 보정
	* 스킬이나 기본 공격의 피해에 대한 추가/감소 비율
	*/
	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing = OnRep_DamageMod)
	FGameplayAttributeData DamageMod;
	ATTRIBUTE_ACCESSORS(UMSPlayerAttributeSet, DamageMod)

	/*
	* 주문 크기 보정
	* 스킬 범위/효과 크기에 대한 비율
	*/
	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing = OnRep_SpellSizeMod)
	FGameplayAttributeData SpellSizeMod;
	ATTRIBUTE_ACCESSORS(UMSPlayerAttributeSet, SpellSizeMod)

	/*
	* 쿨다운 감소
	* 스킬 쿨다운 시간을 감소시키는 비율
	*/
	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing = OnRep_CooldownReduction)
	FGameplayAttributeData CooldownReduction;
	ATTRIBUTE_ACCESSORS(UMSPlayerAttributeSet, CooldownReduction)

	/*
	* 치명타 확률
	* 피해가 치명타로 적용될 확률(0 ~ 1)
	*/
	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing = OnRep_CritChance)
	FGameplayAttributeData CritChance;
	ATTRIBUTE_ACCESSORS(UMSPlayerAttributeSet, CritChance)

	/*
	* 치명타 피해
	* 치명타 시 피해 배수 (ex. 1.5 = 150%)
	*/
	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing = OnRep_CritDamage)
	FGameplayAttributeData CritDamage;
	ATTRIBUTE_ACCESSORS(UMSPlayerAttributeSet, CritDamage)

	/*
	* 획득 반경 보정
	* 아이템/경험치 등의 획득 반경에 대한 추가 비율
	*/
	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing = OnRep_PickupRangeMod)
	FGameplayAttributeData PickupRangeMod;
	ATTRIBUTE_ACCESSORS(UMSPlayerAttributeSet, PickupRangeMod)

	/*
	* 경험치 획득량 보정
	* 추가 경험치 획득 비율
	*/
	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing = OnRep_ExperienceGainMod)
	FGameplayAttributeData ExperienceGainMod;
	ATTRIBUTE_ACCESSORS(UMSPlayerAttributeSet, ExperienceGainMod)

	/*
	* 행운
	* 드랍률이나 랜덤 이벤트에 영향을 미치는 속성
	*/
	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing = OnRep_Luck)
	FGameplayAttributeData Luck;
	ATTRIBUTE_ACCESSORS(UMSPlayerAttributeSet, Luck)

protected:
	// 복제 함수
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 속성 값 변경 콜백 함수
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	/*
	* GAMEPLAYATTRIBUTE_REPNOTIFY 매크로를 사용하는 OnRep 구현
	* 플레이어 캐릭터 속성 리플리케이션 함수
	* 파라미터는 리플리케이트 되기 이전의 속성 값
	*/
	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_HealthRegen(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_Defense(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_DodgeRate(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_MoveSpeedMod(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_DamageMod(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_SpellSizeMod(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_CooldownReduction(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_CritChance(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_CritDamage(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_PickupRangeMod(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_ExperienceGainMod(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_Luck(const FGameplayAttributeData& OldValue);

private:
	// 플레이어 기본 이동 속도 (게임 시작 시 한 번 캐싱)
	float DefaultMovementSpeed = 0.f;
	
	// 플레이어 기본 획득 반경 (게임 시작 시 한 번 캐싱)
	float DefaultPickupRadius = 0.f;

	// 이전 현재 체력 캐시
	float CachedOldHealth = 0.f;

	// 이전 최대 체력 캐시
	float CachedOldMaxHealth = 0.f;
};
