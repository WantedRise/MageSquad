// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/MSBaseEnemy.h"

#include "AbilitySystem/ASC/MSEnemyAbilitySystemComponent.h"
#include "AbilitySystem/AttributeSets/MSEnemyAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AMSBaseEnemy::AMSBaseEnemy()
{
	// Tick 비사용 설정 (사용 시, 활성화)
	// 데칼 영향 비활성화 (사용 시, 활성화)
	//PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	GetMesh()->bReceivesDecals = false;
	
	// 메시의 콜리전은 NoCollision으로 설정
	GetMesh()->SetCollisionProfileName(TEXT("NoCollision"));
	
	// Enemy 전용 콜리전으로 설정
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("MSEnemy"));
	// GetMesh()->SetupAttachment(GetCapsuleComponent());
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -GetCapsuleComponent()->GetScaledCapsuleHalfHeight())); 
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f)); // 예시
	
	// GAS 컴포넌트
	ASC = CreateDefaultSubobject<UMSEnemyAbilitySystemComponent>(TEXT("ASC"));
	ASC->SetIsReplicated(true);  // GAS는 리플리케이트
	AttributeSet = CreateDefaultSubobject<UMSEnemyAttributeSet>(TEXT("AttributeSet"));
	
	// Character는 리플리케이트
	bReplicates = true;
	ACharacter::SetReplicateMovement(true);  // Movement 리플리케이트 (기본값)
    
	// RVO 설정
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->bUseRVOAvoidance = true;
	MoveComp->AvoidanceConsiderationRadius = 500.0f;
	MoveComp->AvoidanceWeight = 0.5f;
	MoveComp->SetAvoidanceGroup(1);
	MoveComp->SetGroupsToAvoidMask(1);        // 그룹 1의 다른 Enemy들을 회피
	MoveComp->SetAvoidanceEnabled(true);      // 명시적 활성화
	
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
}

// Called when the game starts or when spawned
void AMSBaseEnemy::BeginPlay()
{
	Super::BeginPlay();
	if (ASC && !ASC->AbilityActorInfo.IsValid())
	{
		ASC->InitAbilityActorInfo(this, this);
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

UAbilitySystemComponent* AMSBaseEnemy::GetAbilitySystemComponent() const
{
	if (ASC == nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("ASC is nullptr"));
		return nullptr;
	}
	
	return ASC;
}
