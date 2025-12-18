// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_OrientToTarget.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UBTService_OrientToTarget : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTService_OrientToTarget();
	
protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, 
						 uint8* NodeMemory, 
						 float DeltaSeconds) override;
    
	UPROPERTY(EditAnywhere, Category = "AI")
	FBlackboardKeySelector TargetActorKey;
	
	UPROPERTY(EditAnywhere, Category = "AI")
	float RotationInterpSpeed = 0.f;
};
