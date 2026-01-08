// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/MSBossEnemy.h"

#include "MSGameplayTags.h"
#include "AbilitySystem/ASC/MSEnemyAbilitySystemComponent.h"
#include "AIController/MSBossAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/MSDirectionIndicatorComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameStates/MSGameState.h"
#include "Net/UnrealNetwork.h"

AMSBossEnemy::AMSBossEnemy()
{
	bReplicates = true;
	ACharacter::SetReplicateMovement(true);

	// AI Controller 
	static ConstructorHelpers::FClassFinder<AMSBossAIController> NormalEnemyControllerRef(
		TEXT("/Game/Blueprints/Enemies/AI/Boss/BP_BossAIController.BP_BossAIController_C"));
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


	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->AvoidanceWeight = 0.1f;

	DirectionIndicatorComponent = CreateDefaultSubobject<UMSDirectionIndicatorComponent>(TEXT("DirectionTexture"));
}

void AMSBossEnemy::BeginPlay()
{
	Super::BeginPlay();

	SetActorScale3D(FVector(2.f, 2.f, 2.f));

	ASC->AddLooseGameplayTag(MSGameplayTags::Enemy_Tier_Boss);

	if (DirectionIndicatorComponent)
	{
		DirectionIndicatorComponent->bShowDistance = false;
		DirectionIndicatorComponent->bRequiresActivation = false;
	}
}

void AMSBossEnemy::SetPoolingMode(const bool bInPooling)
{
	Super::SetPoolingMode(bInPooling);

	if (AMSBossAIController* AIController = Cast<AMSBossAIController>(GetController()))
	{
		AIController->GetBlackboardComponent()->SetValueAsBool(AIController->GetIsSpawndKey(), !bInPooling);
	}

	DirectionIndicatorComponent->bRequiresActivation = !bInPooling;
}

void AMSBossEnemy::SetPhase2SkeletalMesh(USkeletalMesh* NewSkeletalMesh)
{
	Super::SetPhase2SkeletalMesh(NewSkeletalMesh);

	// 바인딩 시도 함수 호출
	Phase2SkeletalMesh = NewSkeletalMesh;

	UE_LOG(LogTemp, Log, TEXT("Set Phase2 SkeletalMesh"));
}

void AMSBossEnemy::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMSBossEnemy, Phase2SkeletalMesh);
}

void AMSBossEnemy::Multicast_TransitionToPhase2_Implementation()
{
	if (Phase2SkeletalMesh == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Phase2SkeletalMesh is NULL! Check Blueprint settings."));
		return;
	}

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
	if (AMSGameState* GS = Cast<AMSGameState>(GetWorld()->GetGameState()))
	{
		// 서버에서는 이 호출로 NormalAIController들이 반응함 (AI 정지)
		// 클라이언트에서는 이 호출로 카메라 매니저나 UI가 반응함 (연출 시작)
		GS->OnBossSpawnCutsceneStateChanged.Broadcast(bStart);

		UE_LOG(LogTemp, Warning, TEXT("[%s] Multicast_PlaySpawnCutscene_Implementation"),
		       HasAuthority() ? TEXT("Server") : TEXT("Client"));
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	if (bStart)
	{
		bCanOptimization = false; // 스폰 시에는 거리 최적화 x
		// 카메라 전환: 보스 본인(this)을 뷰 타겟으로 설정
		OriginalViewTarget = PC->GetViewTarget();
		PC->SetViewTargetWithBlend(this, 0.5f, VTBlend_Cubic, 0.f, true);
	}
	else
	{
		bCanOptimization = true; // 스폰 시에는 거리 최적화 x
		// 원래 카메라로 복귀
		if (OriginalViewTarget)
		{
			PC->SetViewTargetWithBlend(OriginalViewTarget, 1.0f, VTBlend_Cubic, 0.f, true);
		}

		// Tick 이용 종료
		Camera->SetComponentTickEnabled(false);
		SpringArm->SetComponentTickEnabled(false);
	}
	
	ForceNetUpdate();
}

void AMSBossEnemy::OnRep_Phase2SkeletalMesh(USkeletalMesh* NewSkeletalMesh)
{
	UE_LOG(LogTemp, Warning, TEXT("[CLIENT] OnRep_Phase2SkeletalMesh: %s"), *Phase2SkeletalMesh.GetName());

	if (Phase2SkeletalMesh == nullptr)
	{
		Phase2SkeletalMesh = NewSkeletalMesh;
		UE_LOG(LogTemp, Warning, TEXT("[CLIENT] OnRep_Phase2SkeletalMesh: %s"), *Phase2SkeletalMesh.GetName());
		TrySetMesh(NewSkeletalMesh);
	}
}

UAbilitySystemComponent* AMSBossEnemy::GetIndicatorSourceASC_Implementation() const
{
	return ASC;
}

TSubclassOf<UGameplayEffect> AMSBossEnemy::GetIndicatorDamageEffect_Implementation() const
{
	return DamageEffectClass;
}

void AMSBossEnemy::TrySetMesh(USkeletalMesh* NewSkeletalMesh)
{
	UE_LOG(LogTemp, Warning, TEXT("[CLIENT] TrySetMesh: %s"), *Phase2SkeletalMesh.GetName());

	if (NewSkeletalMesh == nullptr)
	{
		if (Phase2SkeletalMesh == nullptr)
		{
			// 아직 GameState가 NULL이면 다음 프레임에 다시 시도 (성공할 때까지)
			GetWorld()->GetTimerManager().SetTimerForNextTick(
				FTimerDelegate::CreateWeakLambda(this, [this, NewSkeletalMesh]()
				{
					TrySetMesh(NewSkeletalMesh);
				})
			);
		}
	}
	
	else
	{
		Phase2SkeletalMesh = NewSkeletalMesh;
	}
}
