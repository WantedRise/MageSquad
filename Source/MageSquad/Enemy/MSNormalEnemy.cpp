// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/MSNormalEnemy.h"

#include "AIController/MSNormalAIController.h"
#include "GameFramework/CharacterMovementComponent.h"

AMSNormalEnemy::AMSNormalEnemy()
{
	// AI Controller ¼³Á¤
	AIControllerClass = AMSNormalAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	
}

void AMSNormalEnemy::BeginPlay()
{
	Super::BeginPlay();
}

void AMSNormalEnemy::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}