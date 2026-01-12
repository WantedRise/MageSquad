// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/AIController/MSBaseAIController.h"
#include "MSEliteAIController.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API AMSEliteAIController : public AMSBaseAIController
{
	GENERATED_BODY()

	AMSEliteAIController();
	
protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void RunAI() override;
	virtual void StopAI() override;
	
private:
	UFUNCTION()
	void HandleGlobalFreeze(bool bGlobalFreeze);
};
