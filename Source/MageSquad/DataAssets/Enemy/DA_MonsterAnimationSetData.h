// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DA_MonsterAnimationSetData.generated.h"

class UMSEnemyAnimInstance;
/**
 * 작성자 : 임희섭
 * 작성일 : 2025/12/11
 * 일반 몬스터 애니메이션 정보를 담을 데이터 에셋
 */
UCLASS(BlueprintType)
class MAGESQUAD_API UDA_MSMonsterAnimationSet : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anim|Animation Blueprint")
	TSubclassOf<UMSEnemyAnimInstance> AnimationBlueprint;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anim|Montage")
	TObjectPtr<UAnimMontage> IdleAnim;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anim|Montage")
	TObjectPtr<UAnimMontage> WalkAnim;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anim|Montage")
	TObjectPtr<UAnimMontage> AttackAnim;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anim|Montage")
	TObjectPtr<UAnimMontage> DeadAnim;
};