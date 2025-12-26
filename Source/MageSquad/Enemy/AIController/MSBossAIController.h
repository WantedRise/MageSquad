// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/AIController/MSBaseAIController.h"
#include "MSBossAIController.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API AMSBossAIController : public AMSBaseAIController
{
	GENERATED_BODY()

public:
	FORCEINLINE const FName& GetIsSpawndKey() const { return IsSpawndKey; }

private:
	// Blackboard Keys
	UPROPERTY(EditDefaultsOnly, Category = "AI|BlackboardKeys")
	FName IsSpawndKey = "IsSpawnd";
};
