// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/AIController/MSBaseAIController.h"
#include "MSBossAIController.generated.h"

/**
 * 작성자 : 임희섭
 * 작성일 : 2025/12/10
 * 보스몬스터의 AIController
 */
UCLASS()
class MAGESQUAD_API AMSBossAIController : public AMSBaseAIController
{
	GENERATED_BODY()

public:
	FORCEINLINE const FName& GetIsSpawndKey() const { return IsSpawndKey; }
	FORCEINLINE const FName& GetIsGroggyKey() const { return IsGroggyKey; }

private:
	// Blackboard Keys
	UPROPERTY(EditDefaultsOnly, Category = "AI|BlackboardKeys")
	FName IsSpawndKey = "IsSpawnd";
	
	// Blackboard Keys
	UPROPERTY(EditDefaultsOnly, Category = "AI|BlackboardKeys")
	FName IsGroggyKey = "IsGroggy";
};
