// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MSIndicatorDamageInterface.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UMSIndicatorDamageInterface : public UInterface
{
	GENERATED_BODY()
};

/*
 * 작성자 : 임희섭
 * 작성일 : 2026/01/02
 * Indicator가 Fill 완료 시 데미지 정보를 조회하기 위한 인터페이스
 * 보스, 엘리트 등 Indicator를 사용하는 적이 구현
 */
class MAGESQUAD_API IMSIndicatorDamageInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/**
	 * Indicator 데미지에 사용할 ASC 반환
	 * @return 데미지를 가하는 주체의 AbilitySystemComponent
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Indicator")
	UAbilitySystemComponent* GetIndicatorSourceASC() const;

	/**
	 * Indicator 데미지에 사용할 GameplayEffect 클래스 반환
	 * @return 적용할 데미지 GameplayEffect 클래스
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Indicator")
	TSubclassOf<UGameplayEffect> GetIndicatorDamageEffect() const;
};
