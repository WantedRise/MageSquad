// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "MSEnemyAttributeSet.generated.h"

/**
 * 작성자 : 임희섭
 * 작성일 : 2025/12/08
 * 몬스터가 사용할 어트리뷰트셋
 */
UCLASS()
class MAGESQUAD_API UMSEnemyAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UMSEnemyAttributeSet();

	// 속성이 변경되기 전에 호출되는 함수
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	UFUNCTION()
	virtual void OnRep_CurrentHealth(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_MoveSpeed(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_AttackDamage(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_AttackRange(const FGameplayAttributeData& OldValue);

public:
	ATTRIBUTE_ACCESSORS_BASIC(UMSEnemyAttributeSet, CurrentHealth)
	ATTRIBUTE_ACCESSORS_BASIC(UMSEnemyAttributeSet, MaxHealth)
	ATTRIBUTE_ACCESSORS_BASIC(UMSEnemyAttributeSet, MoveSpeed)
	ATTRIBUTE_ACCESSORS_BASIC(UMSEnemyAttributeSet, AttackDamage)
	ATTRIBUTE_ACCESSORS_BASIC(UMSEnemyAttributeSet, AttackRange)
	
protected:
	// 복제 함수
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 속성 값 변경 콜백 함수
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
	
private:
	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing=OnRep_CurrentHealth, meta = (AllowPrivateAccess = "true")) 
	FGameplayAttributeData CurrentHealth;
	
	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing=OnRep_MaxHealth, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData MaxHealth;

	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing=OnRep_MoveSpeed, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData MoveSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing=OnRep_AttackDamage, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData AttackDamage;

	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing=OnRep_AttackRange, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData AttackRange;
		

private:
	// 이전 현재 체력 캐시
	float CachedOldCurrentHealth = 0.f;
};
