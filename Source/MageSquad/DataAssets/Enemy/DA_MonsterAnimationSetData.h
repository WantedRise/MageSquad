// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DA_MonsterAnimationSetData.generated.h"

class UMSEnemyAnimInstance;
/**
 * 작성자 : 임희섭
 * 작성일 : 2025/12/11
 * 몬스터가 가지는 애니메이션을 정의하는 데이터 에셋
 */
UCLASS(BlueprintType)
class MAGESQUAD_API UDA_EnemyAnimationSet : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anim|Animation Blueprint")
	TSubclassOf<UMSEnemyAnimInstance> AnimationClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anim|Montage")
	TObjectPtr<UAnimMontage> IdleAnim;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anim|Montage")
	TObjectPtr<UAnimMontage> WalkAnim;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anim|Montage")
	TObjectPtr<UAnimMontage> AttackAnim;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anim|Montage")
	TObjectPtr<UAnimMontage> DeadAnim;
};