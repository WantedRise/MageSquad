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
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	GetMesh()->bReceivesDecals = false;
	
	// 메시의 콜리전은 NoCollision으로 설정
	GetMesh()->SetCollisionProfileName(TEXT("NoCollision"));
	
	// Enemy 전용 콜리전으로 설정
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("MSEnemy"));
	
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
}

// Called when the game starts or when spawned
void AMSBaseEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	ASC->InitAbilityActorInfo(this, this);
}

void AMSBaseEnemy::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
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
