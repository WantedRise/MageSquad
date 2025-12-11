// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_IsDead.generated.h"

/**
 * 작성자 : 임희섭
 * 작성일 : 2025/12/11
 * 몬스터가 죽었는지 판단합니다. 
 */
UCLASS()
class MAGESQUAD_API UBTDecorator_IsDead : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	UBTDecorator_IsDead();

protected:
	// 데코레이터의 핵심 조건 로직을 구현
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	
protected:
	// 에디터에서 설정할 Blackboard Key
	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector IsDeadKey;
	
};
