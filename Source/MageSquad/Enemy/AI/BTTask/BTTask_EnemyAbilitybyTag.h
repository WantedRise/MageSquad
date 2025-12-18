// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/AI/BTTask/BTTask_EnemyBase.h"
#include "BTTask_EnemyAbilitybyTag.generated.h"

/**
 * 
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
