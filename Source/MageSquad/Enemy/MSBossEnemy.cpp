// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/MSBossEnemy.h"

#include "MSGameplayTags.h"
#include "AbilitySystem/ASC/MSEnemyAbilitySystemComponent.h"
#include "AIController/MSBossAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameStates/MSGameState.h"
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
	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	SpringArm->SocketOffset = FVector(0.f, 0.f, -100.f);
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 4.f;
	SpringArm->TargetArmLength = TargetArmLength;
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = true;
	SpringArm->bInheritYaw = true;
	SpringArm->bDoCollisionTest = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
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
		AIController->GetBlackboardComponent()->SetValueAsBool(AIController->GetIsSpawndKey(), !bInPooling);
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
	//UE_LOG(LogTemp, Warning, TEXT("[CLIENT] Client_TransitionToPhase2_Implementation"));
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

void AMSBossEnemy::Multicast_PlaySpawnCutscene_Implementation(bool bStart)
{
	UE_LOG(LogTemp, Warning, TEXT("[CLIENT] Multicast_PlaySpawnCutscene_Implementation"));
	
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC || !PC->IsLocalController()) return;

	if (bStart)
	{
		// 카메라 전환: 보스 본인(this)을 뷰 타겟으로 설정
		OriginalViewTarget = PC->GetViewTarget();
		PC->SetViewTargetWithBlend(this, 0.5f, VTBlend_Cubic);
	}
	else
	{
		// 원래 카메라로 복귀
		if (OriginalViewTarget)
		{
			PC->SetViewTargetWithBlend(OriginalViewTarget, 1.0f, VTBlend_Cubic);
		}
	}
	
	if (AMSGameState* GS = Cast<AMSGameState>(GetWorld()->GetGameState()))
	{
		// 서버에서는 이 호출로 NormalAIController들이 반응함 (AI 정지)
		// 클라이언트에서는 이 호출로 카메라 매니저나 UI가 반응함 (연출 시작)
		GS->OnBossSpawnCutsceneStateChanged.Broadcast(bStart);
	}
}

void AMSBossEnemy::OnRep_Phase2SkeletalMesh(USkeletalMesh* NewSkeletalMesh)
{
	UE_LOG(LogTemp, Warning, TEXT("[CLIENT] OnRep_Phase2SkeletalMesh: %s"), *Phase2SkeletalMesh.GetName());
	
	Phase2SkeletalMesh = NewSkeletalMesh;
	
}
