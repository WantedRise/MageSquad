// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DA_PlayerStartUpData.generated.h"

/**
 * 플레이어 시작 어빌리티 / 게임플레이 이펙트 구조체
 * Abilities : 플레이어가 소유하게 될 UGameplayAbility 클래스 목록
 * Effects   : 시작 시 한 번 적용되는 UGameplayEffect 클래스 목록
 */
USTRUCT(BlueprintType)
struct MAGESQUAD_API FStartAbilityData
{
	GENERATED_BODY()

public:
	// 플레이어가 시작 시 부여받는 어빌리티 목록
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS | StartUp")
	TArray<TSubclassOf<class UGameplayAbility>> Abilties;

	// 플레이어가 시작 시 한 번 적용되는 게임플레이 이펙트 목록
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS | StartUp")
	TArray<TSubclassOf<class UGameplayEffect>> Effects;
};

/**
 * 작성자: 김준형
 * 작성일: 25/12/08
 *
 * 플레이어 시작 데이터 에셋 클래스
 * 플레이어가 게임 시작 시 부여받는 기본 어빌리티 / 이펙트를 정의하는 데이터 에셋
 */
UCLASS()
class MAGESQUAD_API UDA_PlayerStartUpData : public UDataAsset
{
	GENERATED_BODY()

public:
	// 플레이어 시작 어빌리티/이펙트 데이터
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS | StartUp")
	FStartAbilityData PlayerStartAbilityData;
};
