// Fill out your copyright notice in the Description page of Project Settings.



#include "Enemy/MSNormalEnemy.h"
#include "AbilitySystem/ASC/MSEnemyAbilitySystemComponent.h"
#include "AIController/MSNormalAIController.h"
#include "MSGameplayTags.h"
#include "Components/CapsuleComponent.h"

AMSNormalEnemy::AMSNormalEnemy()
{
	// Character는 리플리케이트
	bReplicates = true;
	ACharacter::SetReplicateMovement(true);  // Movement 리플리케이트 (기본값)
	
	// AI Controller 세팅
	static ConstructorHelpers::FClassFinder<AMSNormalAIController> NormalEnemyControllerRef(TEXT("/Game/Blueprints/Enemies/AI/Normal/BP_NormalAIContoller.BP_NormalAIContoller_C"));
	if (NormalEnemyControllerRef.Succeeded())
	{
		AIControllerClass = NormalEnemyControllerRef.Class;
	}
	
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	
	// @Todo : 스케일 조정 나중에 DataTable에서 반영하도록 변경
	SetActorScale3D(FVector(1.3f,1.3f,1.3f));
	GetCapsuleComponent()->SetCapsuleRadius(100.f);
}

void AMSNormalEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	ASC->AddLooseGameplayTag(MSGameplayTags::Enemy_Tier_Normal);
}

void AMSNormalEnemy::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
}

void AMSNormalEnemy::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}
