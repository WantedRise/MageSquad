// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DA_PlayerStartUpData.generated.h"

/**
 * 플레이어 시작 데이터 에셋 클래스
 * 시작 시, 부여/적용되는 게임플레이 어빌리티/이펙트를 저장
 */
UCLASS()
class MAGESQUAD_API UDA_PlayerStartUpData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	// 어빌리티 부여 함수
	virtual void GiveToAbilitySystemComponent(class UMSPlayerAbilitySystemComponent* InASCToGive);

protected:
	// 어빌리티 부여 함수
	void GrantAbilities(const TArray<TSubclassOf<class UGameplayAbility>>& InAbilities, class UMSPlayerAbilitySystemComponent* InASCToGive);

	// 시작 시 부여 및 활성화(OnGive) 어빌리티
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Custom | StartUpData")
	TArray<TSubclassOf<class UGameplayAbility>> ActivateOnGiveAbilities;

	// 시작 시 부여(OnTriggered) 어빌리티
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Custom | StartUpData")
	TArray<TSubclassOf<class UGameplayAbility>> ReactiveAbilities;

	// 시작 시 적용 게임플레이 이펙트
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Custom | StartUpData")
	TArray<TSubclassOf<class UGameplayEffect>> StartUpGameplayEffects;
};
