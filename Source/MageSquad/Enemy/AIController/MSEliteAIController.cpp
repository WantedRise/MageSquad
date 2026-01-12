// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/AIController/MSEliteAIController.h"

#include "AbilitySystem/AttributeSets/MSEnemyAttributeSet.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "Enemy/MSEliteEnemy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameStates/MSGameState.h"
#include "Navigation/PathFollowingComponent.h"

AMSEliteAIController::AMSEliteAIController()
{
	BehaviorTreeAsset = LoadObject<UBehaviorTree>(nullptr, TEXT("/Game/Blueprints/Enemies/AI/Normal/BT_NormalEnemy.BT_NormalEnemy"));
	BlackBoardAsset = LoadObject<UBlackboardData>(nullptr, TEXT("/Game/Blueprints/Enemies/AI/Normal/BB_NormalEnemy.BB_NormalEnemy"));
}

void AMSEliteAIController::BeginPlay()
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

void AMSEliteAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void AMSEliteAIController::RunAI()
{
	Super::RunAI();
}

void AMSEliteAIController::StopAI()
{
	Super::StopAI();
}

void AMSEliteAIController::HandleGlobalFreeze(bool bGlobalFreeze)
{
	if (bGlobalFreeze)
	{
		// 현재 이동 요청 일시정지
		if (UPathFollowingComponent* PathComp = GetPathFollowingComponent())
		{
			PathComp->PauseMove();
		}
		
		if (AMSEliteEnemy* OwnerEnemy = Cast<AMSEliteEnemy>(GetPawn()))
		{
			OwnerEnemy->GetCharacterMovement()->MaxWalkSpeed = 0.f;
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
		
		if (AMSEliteEnemy* OwnerEnemy = Cast<AMSEliteEnemy>(GetPawn()))
		{
			const UMSEnemyAttributeSet* AttributeSet = Cast<UMSEnemyAttributeSet>(OwnerEnemy->GetAbilitySystemComponent()->GetAttributeSet(UMSEnemyAttributeSet::StaticClass()));
			OwnerEnemy->GetCharacterMovement()->MaxWalkSpeed = AttributeSet->GetMoveSpeed();
		}	
	}
}
