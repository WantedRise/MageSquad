// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/AI/BTTask/BTTask_EnemyBase.h"
#include "BTTask_EnemyAbilitybyTag.generated.h"

/**
 * 작성자 : 임희섭
 * 작성일 : 2025/12/18
 * Enemy들의 Ability를 시작하는 Task
 */
UCLASS()
class MAGESQUAD_API UBTTask_EnemyAbilitybyTag : public UBTTask_EnemyBase
{
	GENERATED_BODY()
	
public:
	UBTTask_EnemyAbilitybyTag();
	
public:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
private:
	UPROPERTY(EditAnywhere, Category = "GAS", meta = (AllowPrivateAccess = "true", Categories = "Enemy.Ability"))
	FGameplayTagContainer GameplayTags;
	
};
