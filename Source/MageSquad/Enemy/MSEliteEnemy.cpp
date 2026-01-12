// Fill out your copyright notice in the Description page of Project Settings.


#include "MSEliteEnemy.h"

#include "MSGameplayTags.h"
#include "AbilitySystem/ASC/MSEnemyAbilitySystemComponent.h"
#include "AIController/MSEliteAIController.h"
#include "Components/CapsuleComponent.h"

AMSEliteEnemy::AMSEliteEnemy()
{
	// Character는 리플리케이트
	bReplicates = true;
	ACharacter::SetReplicateMovement(true);  // Movement 리플리케이트 (기본값)
	
	// AI Controller 세팅
	static ConstructorHelpers::FClassFinder<AMSEliteAIController> EliteEnemyControllerRef(TEXT("/Game/Blueprints/Enemies/AI/Elite/BP_EliteAIContoller.BP_EliteAIContoller_C"));
	if (EliteEnemyControllerRef.Succeeded())
	{
		AIControllerClass = EliteEnemyControllerRef.Class;
	}
	
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	
	// @Todo : 스케일 조정 나중에 DataTable에서 반영하도록 변경
	SetActorScale3D(FVector(2.f,2.f,2.f));
	GetCapsuleComponent()->SetCapsuleRadius(100.f);
}

void AMSEliteEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	ASC->AddLooseGameplayTag(MSGameplayTags::Enemy_Tier_Elite);
}

void AMSEliteEnemy::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

void AMSEliteEnemy::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}
