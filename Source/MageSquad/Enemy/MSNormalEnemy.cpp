// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/MSNormalEnemy.h"
#include "AIController/MSNormalAIController.h"
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
	
	AutoPossessAI = EAutoPossessAI::Disabled;
	
	// @Todo : 스케일 조정 나중에 DataTable에서 반영하도록 변경
	SetActorScale3D(FVector(1.3f,1.3f,1.3f));
}

void AMSNormalEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	// 풀링 중에는 AI Controller 생성 안 함
	if (!bIsInPool && !GetController() && HasAuthority())
	{
		SpawnDefaultController();
	}
}

void AMSNormalEnemy::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
}

void AMSNormalEnemy::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AMSNormalEnemy::SetPoolingMode(bool bInPooling)
{  
	bIsInPool = bInPooling;

	if (UCapsuleComponent* Cap = GetCapsuleComponent())
	{
		if (bInPooling)
		{
			Cap->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Cap->SetGenerateOverlapEvents(false);
		}
		else
		{
			Cap->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			Cap->SetGenerateOverlapEvents(true);

			// 캡슐 오브젝트 타입을 확실히 MSEnemy로 유지
			Cap->SetCollisionObjectType(ECC_GameTraceChannel3); // MSEnemy
		}
	}
}
