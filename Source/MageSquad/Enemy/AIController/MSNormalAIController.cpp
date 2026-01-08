// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/AIController/MSNormalAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "GameStates/MSGameState.h"
#include "Navigation/PathFollowingComponent.h"

AMSNormalAIController::AMSNormalAIController()
{
	BehaviorTreeAsset = LoadObject<UBehaviorTree>(nullptr, TEXT("/Game/Blueprints/Enemies/AI/Normal/BT_NormalEnemy.BT_NormalEnemy"));
	BlackBoardAsset = LoadObject<UBlackboardData>(nullptr, TEXT("/Game/Blueprints/Enemies/AI/Normal/BB_NormalEnemy.BB_NormalEnemy"));
}

void AMSNormalAIController::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		if (AMSGameState* GS = Cast<AMSGameState>(GetWorld()->GetGameState()))
		{
			GS->OnBossSpawnCutsceneStateChanged.AddUObject(this, &ThisClass::HandleGlobalFreeze);
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
		// 현재 이동 요청 일시정지
		if (UPathFollowingComponent* PathComp = GetPathFollowingComponent())
		{
			PathComp->PauseMove();
		}
		
		StopAI();
	}
	else
	{
		RunAI();
		
		// 이동 재개
		if (UPathFollowingComponent* PathComp = GetPathFollowingComponent())
		{
			PathComp->ResumeMove();
		}
	}
}
