// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "DataAssets/Enemy/DA_MonsterAnimationSetData.h"
#include "MSMonsterStaticData.generated.h"

/*
 * 작성자 : 임희섭
 * 작성일 : 2025/12/11
 * 몬스터들의 데이터 정보
 */

USTRUCT(BlueprintType)
struct FMSMonsterStaticData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UDA_MonsterAnimationSet> AnimationSet;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxHealth;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MoveSpeed;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AttackDamage;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AttackRange;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TSubclassOf<UGameplayAbility>> StartAbilities;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TSubclassOf<UGameplayEffect>> StartEffects;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsRanged;
};