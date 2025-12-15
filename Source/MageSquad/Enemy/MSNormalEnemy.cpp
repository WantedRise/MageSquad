// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/MSNormalEnemy.h"
#include "AIController/MSNormalAIController.h"

AMSNormalEnemy::AMSNormalEnemy()
{
	// AI Controller 세팅
	AIControllerClass = AMSNormalAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	
	static ConstructorHelpers::FClassFinder<AMSNormalAIController> NormalEnemyControllerRef(TEXT("/Game/Blueprints/Enemies/AI/BP_NormalAIContoller.BP_NormalAIContoller_C"));
	if (NormalEnemyControllerRef.Succeeded())
	{
		AIControllerClass = NormalEnemyControllerRef.Class;
	}
}

void AMSNormalEnemy::BeginPlay()
{
	Super::BeginPlay();
}

void AMSNormalEnemy::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

void AMSNormalEnemy::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}
