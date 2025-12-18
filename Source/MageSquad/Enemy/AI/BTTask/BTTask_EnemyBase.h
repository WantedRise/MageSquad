// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_EnemyBase.generated.h"

/**
 * 작성자 : 임희섭
 * 작성일 : 2025/12/18
 * Enemy들의 BT Task를 제작할 경우 공통으로 상속받게 할 클래스
 * Owner Enemy를 가지도록 한다. 
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
