// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/MSBaseEnemy.h"

#include "MSGameplayTags.h"
#include "AbilitySystem/ASC/MSEnemyAbilitySystemComponent.h"
#include "AbilitySystem/AttributeSets/MSEnemyAttributeSet.h"
#include "Animation/Enemy/MSEnemyAnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "DataAssets/Enemy/DA_MonsterAnimationSetData.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/MSPlayerCharacter.h"
#include "System/MSEnemySpawnSubsystem.h"

// Sets default values
AMSBaseEnemy::AMSBaseEnemy()
{
	// Tick 비사용 설정 (사용 시, 활성화)
	// 데칼 영향 비활성화 (사용 시, 활성화)
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	GetMesh()->bReceivesDecals = false;
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;

	// 메시의 콜리전은 NoCollision으로 설정
	GetMesh()->SetCollisionProfileName(TEXT("NoCollision"));

	// Enemy 전용 콜리전으로 설정
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("MSEnemy"));
	// GetMesh()->SetupAttachment(GetCapsuleComponent());
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

	// GAS 컴포넌트
	ASC = CreateDefaultSubobject<UMSEnemyAbilitySystemComponent>(TEXT("ASC"));
	ASC->SetIsReplicated(true); // GAS는 리플리케이트
	AttributeSet = CreateDefaultSubobject<UMSEnemyAttributeSet>(TEXT("AttributeSet"));
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	// Character는 리플리케이트
	bReplicates = true;
	ACharacter::SetReplicateMovement(true); // Movement 리플리케이트 (기본값)
	bNetLoadOnClient = true;
	bAlwaysRelevant = false;
	SetNetCullDistanceSquared(0.f); // 리플레케이션 거리 무한 // 스폰할 때 무한으로
	SetNetUpdateFrequency(100.f);
	NetDormancy = DORM_Initial; 

	// RVO 설정
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->bUseRVOAvoidance = true;
	MoveComp->AvoidanceConsiderationRadius = 500.0f;
	MoveComp->AvoidanceWeight = 0.5f;
	MoveComp->SetAvoidanceGroup(1);
	MoveComp->SetGroupsToAvoidMask(1); // 그룹 1의 다른 Enemy들을 회피
	MoveComp->SetAvoidanceEnabled(true); // 명시적 활성화

	// 기본 회전 설정
	MoveComp->bOrientRotationToMovement = true;
	MoveComp->bUseControllerDesiredRotation = false;
	MoveComp->RotationRate = FRotator(0.0f, 270.0f, 0.0f); // 중간 속도

	// 가속/감속으로 자연스러운 움직임
	MoveComp->MaxAcceleration = 800.0f;
	MoveComp->BrakingDecelerationWalking = 800.0f;
	MoveComp->MaxWalkSpeed = 400.0f;

	// 곡선 이동을 위한 설정
	MoveComp->bRequestedMoveUseAcceleration = true;

	// 액터 태그 설정
	Tags.AddUnique(TEXT("Enemy"));

	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AMSBaseEnemy::OnCapsuleBeginOverlap);

	static ConstructorHelpers::FClassFinder<UGameplayEffect> DamageEffectClassRef(
		TEXT("/Game/Blueprints/GAS/GE/BPGE_EnemyCollisionDamage.BPGE_EnemyCollisionDamage_C"));

	if (DamageEffectClassRef.Succeeded())
	{
		DamageEffectClass = DamageEffectClassRef.Class;
	}

	static ConstructorHelpers::FClassFinder<UGameplayEffect> CooldownEffectClassRef(
		TEXT("/Game/Blueprints/GAS/GE/BPGE_EnemyPatternCooldown.BPGE_EnemyPatternCooldown_C"));

	if (CooldownEffectClassRef.Succeeded())
	{
		CooldownEffectClass = CooldownEffectClassRef.Class;
	}
}

// Called when the game starts or when spawned
void AMSBaseEnemy::BeginPlay()
{
	Super::BeginPlay();

	// ENetMode NetMode = GetWorld()->GetNetMode();
	// UE_LOG(LogTemp, Error, TEXT("[Enemy BeginPlay] %s - NetMode: %d, HasAuthority: %s"), 
	// 	*GetName(),
	// 	(int32)NetMode,
	// 	HasAuthority() ? TEXT("Server") : TEXT("Client"));

	if (!HasAuthority() && !CurrentMonsterID.IsNone())
	{
		OnRep_MonsterID();
	}

	if (ASC && !ASC->AbilityActorInfo.IsValid())
	{
		ASC->InitAbilityActorInfo(this, this);
	}

	auto* SigManager = USignificanceManager::Get(GetWorld());
	if (SigManager)
	{
		// "Enemy"라는 태그로 자신을 등록하고, 위에서 만든 함수를 연결
		SigManager->RegisterObject(
			this,
			"Enemy",
			&AMSBaseEnemy::CalculateSignificance, // 여기에 만든 함수 위치를 전달
			USignificanceManager::EPostSignificanceType::Sequential,
			[this](USignificanceManager::FManagedObjectInfo* ObjectInfo, float OldSig, float NewSig, bool bInView)
			{
				// 중요도가 변했을 때 실행될 로직 (예: 틱 빈도 조절)
				this->OnSignificanceChanged(ObjectInfo, OldSig, NewSig, bInView);
			}
		);
	}
}

void AMSBaseEnemy::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

void AMSBaseEnemy::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AMSBaseEnemy::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//  MonsterID를 모든 클라이언트에 Replicate
	DOREPLIFETIME_CONDITION_NOTIFY(AMSBaseEnemy, CurrentMonsterID, COND_None, REPNOTIFY_Always);
}

bool AMSBaseEnemy::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget,
                                    const FVector& SrcLocation) const
{
	// 항상 Relevant (풀링된 비활성 액터도 클라이언트에 존재해야 함)
	//UE_LOG(LogTemp, Warning, TEXT("[IsNetRelevantFor] %s called"), *GetName());
	return true;
}

UAbilitySystemComponent* AMSBaseEnemy::GetAbilitySystemComponent() const
{
	if (ASC == nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("ASC is nullptr"));
		return nullptr;
	}

	return ASC;
}

void AMSBaseEnemy::OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                         const FHitResult& SweepResult)
{
	if (AMSPlayerCharacter* Player = Cast<AMSPlayerCharacter>(OtherActor))
	{
		UAbilitySystemComponent* TargetASC = Player->GetAbilitySystemComponent();
		// GameplayEffectSpec 생성
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		Context.AddSourceObject(this);
		Context.AddHitResult(SweepResult);

		FGameplayEffectSpecHandle SpecHandle =
			ASC->MakeOutgoingSpec(DamageEffectClass, 1.f, Context);

		if (!SpecHandle.IsValid())
		{
			return;
		}

		// @Todo : 시간이 지날수록 쎄짐
		// @Todo : 플레이어 방어력도 계산해야됨
		SpecHandle.Data->SetSetByCallerMagnitude(
			MSGameplayTags::Data_Damage,
			-AttributeSet->GetAttackDamage()
		);

		// GameplayEffect 적용
		TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
	}
}

void AMSBaseEnemy::SetMonsterID(const FName& NewMonsterID)
{
	if (CurrentMonsterID == NewMonsterID)
	{
		return;
	}

	// 	UE_LOG(LogTemp, Error, TEXT("[SetMonsterID] %s | Old: %s -> New: %s | HasAuth: %d"), 
	// *GetName(),
	// *CurrentMonsterID.ToString(),
	// *NewMonsterID.ToString(),
	// HasAuthority());

	CurrentMonsterID = NewMonsterID;

	if (HasAuthority())
	{
		ForceNetUpdate();
	}
}

void AMSBaseEnemy::SetAnimData(UDA_EnemyAnimationSet* NewAnimData)
{
	AnimData = NewAnimData;
	GetMesh()->SetAnimInstanceClass(AnimData->AnimationClass);
}

void AMSBaseEnemy::OnRep_MonsterID()
{
	// 클라이언트에서 MonsterID 변경 시 자동 호출됨!
	UE_LOG(LogTemp, Warning, TEXT("[CLIENT] OnRep_MonsterID: %s"), *CurrentMonsterID.ToString());

	if (CurrentMonsterID == NAME_None)
	{
		return;
	}

	// Subsystem에서 캐시된 데이터 가져오기
	if (UMSEnemySpawnSubsystem* SpawnSystem = UMSEnemySpawnSubsystem::Get(this))
	{
		UE_LOG(LogTemp, Warning, TEXT("[CLIENT] OnRep_MonsterID: for %s"), *CurrentMonsterID.ToString());
		//  클라이언트에서도 InitializeEnemyFromData 호출
		SpawnSystem->ActivateEnemy(this);
		SpawnSystem->InitializeEnemyFromData(this, CurrentMonsterID);
	}
}

float AMSBaseEnemy::CalculateSignificance(USignificanceManager::FManagedObjectInfo* ObjectInfo,
                                          const FTransform& Viewpoint)
{
	AActor* Owner = Cast<AActor>(ObjectInfo->GetObject());
	if (!Owner)
	{
		return 0.0f;
	}

	float DistanceSq = FVector::DistSquared(Owner->GetActorLocation(), Viewpoint.GetLocation());

	// 20미터 이내: 1.0, 40미터 이내: 0.5, 그 외: 0.1
	if (DistanceSq < FMath::Square(2000.0f))
	{
		return 1.0f;
	}
	if (DistanceSq < FMath::Square(4000.0f))
	{
		return 0.5f;
	}

	return 0.1f;
}

void AMSBaseEnemy::OnSignificanceChanged(USignificanceManager::FManagedObjectInfo* ObjectInfo, float OldSig,
                                         float NewSig, bool bInView)
{
	if (NewSig <= 0.1f)
	{
		if (GetMesh())
		{
			GetMesh()->SetComponentTickEnabled(false);
		}

		// 콜리전 변경, 오버랩 이벤트, Movement Tick 간격 등 게임 플레이에 관련된 부분은 서버에서만 처리
		if (HasAuthority())
		{
			if (UCapsuleComponent* Cap = GetCapsuleComponent())
			{
				Cap->SetGenerateOverlapEvents(false);
			}

			GetCharacterMovement()->PrimaryComponentTick.TickInterval = 0.25f;
		}
	}
	else
	{
		if (GetMesh())
		{
			GetMesh()->SetComponentTickEnabled(true);
		}

		if (HasAuthority())
		{
			if (UCapsuleComponent* Cap = GetCapsuleComponent())
			{
				Cap->SetGenerateOverlapEvents(true);
			}

			GetCharacterMovement()->PrimaryComponentTick.TickInterval = 0.0f;
		}
	}
}

void AMSBaseEnemy::SetPoolingMode(bool bInPooling)
{
	bIsInPool = bInPooling;

	if (UCapsuleComponent* Cap = GetCapsuleComponent())
	{
		if (bInPooling)
		{
			Cap->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Cap->SetGenerateOverlapEvents(false);

			// if (GetCharacterMovement())
			// {
			// 	GetCharacterMovement()->PrimaryComponentTick.SetTickFunctionEnable(false);
			// }
		}
		else
		{
			Cap->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			Cap->SetGenerateOverlapEvents(true);

			// 캡슐 오브젝트 타입을 확실히 MSEnemy로 유지
			Cap->SetCollisionObjectType(ECC_GameTraceChannel3); // MSEnemy

			// if (GetCharacterMovement())
			// {
			// 	GetCharacterMovement()->PrimaryComponentTick.SetTickFunctionEnable(true);
			// }
		}
	}
}
