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
	AIControllerClass = AMSNormalAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::Disabled;
	
	static ConstructorHelpers::FClassFinder<AMSNormalAIController> NormalEnemyControllerRef(TEXT("/Game/Blueprints/Enemies/AI/BP_NormalAIContoller.BP_NormalAIContoller_C"));
	if (NormalEnemyControllerRef.Succeeded())
	{
		AIControllerClass = NormalEnemyControllerRef.Class;
	}
}

void AMSNormalEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	// ✅ 풀링 중에는 AI Controller 생성 안 함
	if (!bIsInPool && !GetController() && HasAuthority())
	{
		SpawnDefaultController();
	}
}

void AMSNormalEnemy::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	// UE_LOG(LogTemp, Error, TEXT("=== NETWORK INFO ==="));
	// UE_LOG(LogTemp, Error, TEXT("World NetMode: %d"), (int32)GetWorld()->GetNetMode());
	// UE_LOG(LogTemp, Error, TEXT("HasAuthority: %d"), HasAuthority());
	// UE_LOG(LogTemp, Error, TEXT("LocalRole: %d (%s)"), 
	// 	(int32)GetLocalRole(),
	// 	*UEnum::GetValueAsString(GetLocalRole())  // ← 이름 출력!
	// );
	// UE_LOG(LogTemp, Error, TEXT("RemoteRole: %d (%s)"), 
	// 	(int32)GetRemoteRole(),
	// 	*UEnum::GetValueAsString(GetRemoteRole())  // ← 이름 출력!
	// );
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
