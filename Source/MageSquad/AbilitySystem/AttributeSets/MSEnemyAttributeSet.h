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

	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_MoveSpeed(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_AttackDamage(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_AttackRange(const FGameplayAttributeData& OldValue);

public:
	ATTRIBUTE_ACCESSORS_BASIC(UMSEnemyAttributeSet, Health)
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
	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing=OnRep_Health, meta = (AllowPrivateAccess = "true")) 
	FGameplayAttributeData Health;
	
	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing=OnRep_MaxHealth, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData MaxHealth;

	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing=OnRep_MoveSpeed, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData MoveSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing=OnRep_AttackDamage, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData AttackDamage;

	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing=OnRep_AttackRange, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData AttackRange;
		
};
