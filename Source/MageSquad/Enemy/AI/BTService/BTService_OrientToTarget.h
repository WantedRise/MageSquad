// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_OrientToTarget.generated.h"

/**
 * 작성자 : 임희섭
 * 작성일 : 2025/12/18
 * AIPawn이 Target을 향해 회전하도록 해주는 Service
 * 기본 상태에서는 무조건 타겟을 향해 바라보며 이동하기 때문에 해당 Service는 UpdateTarget에서 따로 분리
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
