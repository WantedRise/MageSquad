// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "TimerManager.h"
#include "MSEnemyAttributeSet.generated.h"

/**
 * 작성자 : 임희섭
 * 작성일 : 2025/12/12
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
	
	UFUNCTION()
	virtual void OnRep_DropExpValue(const FGameplayAttributeData& OldValue);

public:
	ATTRIBUTE_ACCESSORS_BASIC(UMSEnemyAttributeSet, CurrentHealth)
	ATTRIBUTE_ACCESSORS_BASIC(UMSEnemyAttributeSet, MaxHealth)
	ATTRIBUTE_ACCESSORS_BASIC(UMSEnemyAttributeSet, MoveSpeed)
	ATTRIBUTE_ACCESSORS_BASIC(UMSEnemyAttributeSet, AttackDamage)
	ATTRIBUTE_ACCESSORS_BASIC(UMSEnemyAttributeSet, AttackRange)
	ATTRIBUTE_ACCESSORS_BASIC(UMSEnemyAttributeSet, DropExpValue)
	
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
	
	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing=OnRep_DropExpValue, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData DropExpValue;
		


	/*
	* 김준형
	* 받은 피해량 출력 이벤트 전달 로직 구현
	*/
private:
	// 이전 현재 체력 캐시
	float CachedOldCurrentHealth = 0.f;

private:
	// 받은 피해량 누적 함수
	void QueueDamageFloater(float DeltaHealth, bool bIsCritical);

	// 피해량 전송 함수
	void FlushDamageFloater();

private:
	// 피해량 전송 주기
	static constexpr float DamageFloaterBatchInterval = 0.1f;

	// 누적된 받은 피해량
	float PendingDamageFloater = 0.f;

	// 피해량 전송 주기동안 치명타를 받았는지 여부
	bool bPendingCritical = false;

	// 피해량 전송 델리게이트
	FTimerHandle DamageFloaterFlushHandle;
};
