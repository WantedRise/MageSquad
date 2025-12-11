// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MSBaseAIController.generated.h"

/**
 * 작성자 : 임희섭
 * 작성일 : 2025/12/10
 * 몬스터들의 Base AI Controller 클래스
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
	FORCEINLINE FName GetTargetActorKey() const { return TargetActorKey; }
	FORCEINLINE FName GetTargetLocationKey() const { return TargetLocationKey; }
	FORCEINLINE FName GetCanAttackKey() const { return CanAttackKey; }
	FORCEINLINE FName GetIsDeadKey() const { return IsDeadKey; }	

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
	FName CanAttackKey = "CanAttack";

	UPROPERTY(EditDefaultsOnly, Category = "AI|BlackboardKeys")
	FName IsDeadKey = "IsDead";
};