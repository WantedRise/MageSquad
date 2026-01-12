// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_CheckHasTag.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UBTDecorator_CheckHasTag : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	UBTDecorator_CheckHasTag();
	
protected:
	// Decorator 리턴 값을 체크하는 함수
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	
private:
	UPROPERTY(EditAnywhere, Category = "GAS", meta = (AllowPrivateAccess = "true", Categories = "Enemy.State"))
	FGameplayTagContainer GameplayTags;
};
