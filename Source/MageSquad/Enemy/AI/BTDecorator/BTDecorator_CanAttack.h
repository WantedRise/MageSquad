// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_CanAttack.generated.h"

/**
 * 작성자 : 임희섭
 * 작성일 : 2025/12/12
 * 타겟이 Attack 가능한 범위 안에 들어왔는지 확인하는 Decorator
 */
UCLASS()
class MAGESQUAD_API UBTDecorator_CanAttack : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_CanAttack();
	
protected:
	// Decorator 리턴 값을 체크하는 함수
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

protected:
	// Blackboard Key
	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector CanAttackKey;
};
