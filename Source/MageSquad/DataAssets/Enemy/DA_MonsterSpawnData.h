// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Engine/DataAsset.h"
#include "DA_MonsterSpawnData.generated.h"

/**
 * 작성자 : 임희섭
 * 작성일 : 2025/12/11
 * 몬스터 스폰에 이용될 데이터 목록
 */

UCLASS(BlueprintType)
class MAGESQUAD_API UDA_MonsterSpawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName MonsterID;
    
	// UPROPERTY(EditAnywhere, BlueprintReadOnly)
	// EMonsterTier Tier; // Normal, Elite, Boss
	//    
	// 미리 로드된 에셋들
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMesh> SkeletalMesh;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UDA_MonsterSpawnData> AnimationSet;
    
	// UPROPERTY(EditAnywhere, BlueprintReadOnly)
	// FMSMonsterStats BaseStats;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="Tier != EMonsterTier::Normal"))
	TArray<TSubclassOf<UGameplayAbility>> StartAbilities;
	
};
