// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/MSBossEnemy.h"

#include "MSGameplayTags.h"
#include "AbilitySystem/ASC/MSEnemyAbilitySystemComponent.h"
#include "AIController/MSBossAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Net/UnrealNetwork.h"

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
	
	ASC->AddLooseGameplayTag(MSGameplayTags::Enemy_Tier_Boss);
}

void AMSBossEnemy::SetPoolingMode(const bool bInPooling)
{
	Super::SetPoolingMode(bInPooling);
	
	if (AMSBossAIController* AIController = Cast<AMSBossAIController>(GetController()))
	{
		//AIController->GetBlackboardComponent()->SetValueAsBool(AIController->GetIsSpawndKey(), !bInPooling);
		AIController->GetBlackboardComponent()->SetValueAsBool(AIController->GetIsGroggyKey(), !bInPooling);
	}
}

void AMSBossEnemy::SetPhase2SkeletalMesh(USkeletalMesh* NewSkeletalMesh)
{
	Super::SetPhase2SkeletalMesh(NewSkeletalMesh);
	
	Phase2SkeletalMesh = NewSkeletalMesh;
	
	UE_LOG(LogTemp, Log, TEXT("Set Phase2 SkeletalMesh"));
}

void AMSBossEnemy::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AMSBossEnemy, Phase2SkeletalMesh);
}

void AMSBossEnemy::NetMulticast_TransitionToPhase2_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("[CLIENT] Client_TransitionToPhase2_Implementation"));
	GetMesh()->SetSkeletalMesh(Phase2SkeletalMesh);
	
	// 머티리얼 정보 설정
	const TArray<FSkeletalMaterial>& MeshMaterials = Phase2SkeletalMesh->GetMaterials();
	for (int32 i = 0; i < MeshMaterials.Num(); ++i)
	{
		if (MeshMaterials[i].MaterialInterface)
		{
			GetMesh()->SetMaterial(i, MeshMaterials[i].MaterialInterface);
		}
	}

	// 렌더링 상태 업데이트
	GetMesh()->MarkRenderStateDirty();
}

void AMSBossEnemy::OnRep_Phase2SkeletalMesh(USkeletalMesh* NewSkeletalMesh)
{
	UE_LOG(LogTemp, Warning, TEXT("[CLIENT] OnRep_Phase2SkeletalMesh: %s"), *Phase2SkeletalMesh.GetName());
	
	Phase2SkeletalMesh = NewSkeletalMesh;
	
}
