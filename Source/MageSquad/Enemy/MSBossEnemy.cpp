// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/MSBossEnemy.h"

#include "AIController/MSBossAIController.h"
#include "BehaviorTree/BlackboardComponent.h"

AMSBossEnemy::AMSBossEnemy()
{	
	bReplicates = true;
	ACharacter::SetReplicateMovement(true);
	
	// AI Controller 
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

void AMSBossEnemy::SetPoolingMode(bool bInPooling)
{
	Super::SetPoolingMode(bInPooling);
	
	if (AMSBossAIController* AIController = Cast<AMSBossAIController>(GetController()))
	{
		AIController->GetBlackboardComponent()->SetValueAsBool(TEXT("IsSpawnd"), !bInPooling);		
	}
}
