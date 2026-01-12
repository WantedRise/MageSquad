// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MSBaseAIController.generated.h"

/**
 * 작성자 : 임희섭
 * 작성일 : 2025/12/10
 * AIController의 Base 클래스, 공통 기능을 가진다. 
 */
UCLASS()
class MAGESQUAD_API AMSBaseAIController : public AAIController
{
	GENERATED_BODY()

public:
	AMSBaseAIController();

public:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void RunAI();
	virtual void StopAI();

public:
	FORCEINLINE const FName& GetTargetActorKey() const { return TargetActorKey; }
	FORCEINLINE const FName& GetTargetLocationKey() const { return TargetLocationKey; }
	FORCEINLINE const FName& GetTargetDistanceKey() const { return TargetDistanceKey; }
	FORCEINLINE const FName& GetCanAttackKey() const { return CanAttackKey; }
	FORCEINLINE const FName& GetIsDeadKey() const { return IsDeadKey; }	

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UBlackboardData> BlackBoardAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UBehaviorTree> BehaviorTreeAsset;

	// Blackboard Keys
	UPROPERTY(EditDefaultsOnly, Category = "AI|BlackboardKeys")
	FName TargetActorKey = "TargetActor";

	UPROPERTY(EditDefaultsOnly, Category = "AI|BlackboardKeys")
	FName TargetLocationKey = "TargetLocation";

	UPROPERTY(EditDefaultsOnly, Category = "AI|BlackboardKeys")
	FName TargetDistanceKey = "TargetDistance";
	
	UPROPERTY(EditDefaultsOnly, Category = "AI|BlackboardKeys")
	FName CanAttackKey = "CanAttack";

	UPROPERTY(EditDefaultsOnly, Category = "AI|BlackboardKeys")
	FName IsDeadKey = "IsDead";

	
};