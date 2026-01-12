// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateTarget.generated.h"

/**
 * 작성자 : 임희섭
 * 작성일 : 2025/12/17
 * AI의 Target을 Update해주는 Service
 * 레벨에 스폰된 Player Actor 중 가장 가까운 Player를 Target으로 지정
 */
UCLASS()
class MAGESQUAD_API UBTService_UpdateTarget : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTService_UpdateTarget();
    
protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, 
						 uint8* NodeMemory, 
						 float DeltaSeconds) override;
    
	UPROPERTY(EditAnywhere, Category = "AI")
	FBlackboardKeySelector TargetActorKey;
	
};
