// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/AIController/MSBaseAIController.h"
#include "MSNormalAIController.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API AMSNormalAIController : public AMSBaseAIController
{
	GENERATED_BODY()
	
public:
	AMSNormalAIController();
	
public:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void RunAI();
	virtual void StopAI();
	
};
