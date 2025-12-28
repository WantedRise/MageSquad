// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/AIController/MSBaseAIController.h"
#include "MSNormalAIController.generated.h"

/**
 * 작성자 : 임희섭
 * 작성일 : 2025/12/11
 * 일반 몬스터 AI Controller
 */
UCLASS()
class MAGESQUAD_API AMSNormalAIController : public AMSBaseAIController
{
	GENERATED_BODY()
	
public:
	AMSNormalAIController();
	
public:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void RunAI() override;
	virtual void StopAI() override;
	
private:
	UFUNCTION()
	void HandleGlobalFreeze(bool bGlobalFreeze);
	
};
