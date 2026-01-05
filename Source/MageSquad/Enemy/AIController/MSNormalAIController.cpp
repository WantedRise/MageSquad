// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/AIController/MSNormalAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "GameStates/MSGameState.h"

AMSNormalAIController::AMSNormalAIController()
{
	BehaviorTreeAsset = LoadObject<UBehaviorTree>(nullptr, TEXT("/Game/Blueprints/Enemies/AI/BT_NormalEnemy.BT_NormalEnemy"));
	BlackBoardAsset = LoadObject<UBlackboardData>(nullptr, TEXT("/Game/Blueprints/Enemies/AI/BB_NormalEnemy.BB_NormalEnemy"));
}

void AMSNormalAIController::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		if (AMSGameState* GS = Cast<AMSGameState>(GetWorld()->GetGameState()))
		{
			GS->OnBossSpawnCutsceneStateChanged.AddDynamic(this, &ThisClass::HandleGlobalFreeze);
		}
	}
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

void AMSNormalAIController::HandleGlobalFreeze(bool bGlobalFreeze)
{
	if (bGlobalFreeze)
	{
		StopAI();
	}
	else
	{
		RunAI();
	}
}
