// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataAssets/Enemy/DA_EnemyAbilityData.h"
#include "DataAssets/Enemy/DA_EnemyStaticSoundData.h"
#include "DataAssets/Enemy/DA_MonsterAnimationSetData.h"
#include "MSEnemyStaticData.generated.h"

/*
 * 작성자 : 임희섭
 * 작성일 : 2025/12/11
 * 몬스터들의 데이터 정보
 */

USTRUCT(BlueprintType)
struct FMSEnemyStaticData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UDA_EnemyAnimationSet> AnimationSet;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxHealth;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MoveSpeed;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AttackDamage;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AttackRange;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UDA_EnemyAbilityData> EnemyAbilities;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsRanged;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DropExpValue;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<class UProjectileStaticData> ProjectileDataClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<USkeletalMesh> Phase2SkeletalMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> IndicatorImage;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UDA_EnemyStaticSoundData> EnemySounds;
};