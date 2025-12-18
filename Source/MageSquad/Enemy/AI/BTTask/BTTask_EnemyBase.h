// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_EnemyBase.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UBTTask_EnemyBase : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
protected:
	TObjectPtr<class AMSBaseEnemy> OwnerEnemy;
	
};
