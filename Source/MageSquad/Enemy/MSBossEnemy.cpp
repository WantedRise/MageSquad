// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/MSBossEnemy.h"

#include "AIController/MSBossAIController.h"

AMSBossEnemy::AMSBossEnemy()
{	
	// Character는 리플리케이트
	bReplicates = true;
	ACharacter::SetReplicateMovement(true);  // Movement 리플리케이트 (기본값)
	
	// AI Controller 세팅
	static ConstructorHelpers::FClassFinder<AMSBossAIController> NormalEnemyControllerRef(TEXT("/Game/Blueprints/Enemies/AI/Boss/BP_BossAIController.BP_BossAIController_C"));
	if (NormalEnemyControllerRef.Succeeded())
	{
		AIControllerClass = NormalEnemyControllerRef.Class;
	}	
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AMSBossEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	SetActorScale3D(FVector(3.f, 3.f, 3.f));	
}
