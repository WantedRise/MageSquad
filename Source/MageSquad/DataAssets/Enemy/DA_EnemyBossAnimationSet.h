// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataAssets/Enemy/DA_MonsterAnimationSetData.h"
#include "DA_EnemyBossAnimationSet.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UDA_EnemyBossAnimationSet : public UDA_EnemyAnimationSet
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anim|Montage")
	TObjectPtr<UAnimMontage> Partten1Anim;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anim|Montage")
	TObjectPtr<UAnimMontage> Partten2Anim;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anim|Montage")
	TObjectPtr<UAnimMontage> Partten3Anim;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anim|Montage")
	TObjectPtr<UAnimMontage> SpawnAnim;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anim|Montage")
	TObjectPtr<UAnimMontage> GroggyAnim;
};
