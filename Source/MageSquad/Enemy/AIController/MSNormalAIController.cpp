// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/AIController/MSNormalAIController.h"
#include "BehaviorTree/BehaviorTree.h"

AMSNormalAIController::AMSNormalAIController()
{
	BehaviorTreeAsset = LoadObject<UBehaviorTree>(nullptr, TEXT("/Game/Blueprints/Enemies/AI/BT_NormalEnemy.BT_NormalEnemy"));
}

void AMSNormalAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void AMSNormalAIController::RunAI()
{
	Super::RunAI();
}

void AMSNormalAIController::StopAI()
{
	Super::StopAI();
}