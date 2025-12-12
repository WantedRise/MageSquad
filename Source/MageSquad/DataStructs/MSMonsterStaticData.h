// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "DataAssets/Enemy/DA_MonsterAnimationSetData.h"
#include "MSMonsterStaticData.generated.h"

/*
 * 작성자 : 임희섭
 * 작성일 : 2025/12/11
 * 몬스터들이 공통으로 필요로 하는 데이터 모음
 */

USTRUCT(BlueprintType)
struct FMSMonsterStaticData : public FTableRowBase
{
	GENERATED_BODY()
    
	// UPROPERTY(EditAnywhere, BlueprintReadOnly)
	// FName MonsterID;
    
	// 비주얼
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UDA_MSMonsterAnimationSet> AnimationSet;
    
	// 기본 스탯
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxHealth;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MoveSpeed;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AttackDamage;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AttackRange;
    
	// GAS 관련 (Elite/Boss용)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TSubclassOf<UGameplayAbility>> StartAbilities;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TSubclassOf<UGameplayEffect>> StartEffects;
    
	// 타입 구분
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsRanged;
};