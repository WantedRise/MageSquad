// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/MSNormalEnemy.h"

#include "AbilitySystem/AttributeSets/MSEnemyAttributeSet.h"
#include "AIController/MSNormalAIController.h"
#include "Animation/Enemy/MSEnemyAnimInstance.h"
#include "DataAssets/Enemy/DA_MonsterAnimationSetData.h"
#include "DataAssets/Enemy/DA_MonsterSpawnData.h"
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

void AMSNormalEnemy::InitEnemyData(UDA_MonsterSpawnData* InEnemyData)
{
	Super::InitEnemyData(InEnemyData);
	
	if (InEnemyData == nullptr)
	{
		return;
	}
	
	EnemyData = InEnemyData;
	
	GetMesh()->SetSkeletalMesh(EnemyData->SkeletalMesh);
	GetMesh()->SetAnimInstanceClass(EnemyData->AnimationSet->AnimationClass);
	
	// AttributeSet->SetMaxHealth(EnemyData->);
	// AttributeSet->SetAttackDamage(EnemyData->)
	// AttributeSet->SetAttackRange(EnemyData->);
	// AttributeSet->SetMaxHealth(EnemyData->);
	// AttributeSet->SetMoveSpeed(EnemyData->);
	
}
