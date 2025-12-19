// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "AbilitySystemComponent.h"
#include "MSMVVM_PlayerViewModel.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UMSMVVM_PlayerViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter)
	float Health;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter)
	float MaxHealth;
    
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter)
	float CurrentHealth;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter)
	bool bIsLowHealth;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter)
	float DamageOverlayAlpha;

public:
	// ASC 초기화
	void InitializeWithASC(UAbilitySystemComponent* ASC);
    
	// 콜백 해제
	void UninitializeFromASC();
	
public:
	// Alpha 값을 기반으로 최종 LinearColor를 만들어주는 변환 함수
	UFUNCTION(BlueprintPure, Category = "MVVM|Conversion")
	FLinearColor GetDamageOverlayColor() const;

private:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> CachedASC;

	FDelegateHandle HealthChangedDelegateHandle;
	FDelegateHandle MaxHealthChangedDelegateHandle;

	// GAS Attribute 변경 콜백
	void OnHealthChanged(const FOnAttributeChangeData& Data);
	void OnMaxHealthChanged(const FOnAttributeChangeData& Data);
    
	// Setter 함수들
	void SetHealth(float NewHealth);
	float GetHealth() const { return Health; }

	void SetMaxHealth(float NewMaxHealth);
	float GetMaxHealth() const { return MaxHealth; }

	void SetCurrentHealth(float NewCurrentHealth);
	float GetCurrentHealth() const { return CurrentHealth; }

	void SetbIsLowHealth(bool bNewIsLowHealth);
	bool GetbIsLowHealth() const { return bIsLowHealth; }

	void SetDamageOverlayAlpha(float NewAlpha);
	float GetDamageOverlayAlpha() const { return DamageOverlayAlpha; }

	// 내부 헬퍼
	void UpdateHealthValues();
	void UpdateLowHealthState();

	// 깜빡임 관련
	FTimerHandle BlinkTimerHandle;
	bool bBlinkIncreasing = true;
	float BlinkSpeed = 2.0f;

	void UpdateDamageOverlayBlink();
	void StartBlinking();
	void StopBlinking();
	
};
