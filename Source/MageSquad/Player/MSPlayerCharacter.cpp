// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MSPlayerCharacter.h"
//#include "Engine/LocalPlayer.h"
//#include "Camera/CameraComponent.h"
//#include "Components/CapsuleComponent.h"
//
//#include "GameFramework/SpringArmComponent.h"
//#include "GameFramework/Controller.h"
//#include "GameFramework/CharacterMovementComponent.h"
//
//#include "EnhancedInputComponent.h"
//#include "EnhancedInputSubsystems.h"
//
//#include "DataAssets/Player/DA_PlayerStartUpData.h"
//
//#include "AbilitySystem/ASC/MS_PlayerAbilitySystemComponent.h"
//#include "AbilitySystem/AttributeSets/MS_PlayerAttributeSet.h"
//
//#include "Net/UnrealNetwork.h"
//
//AMS_PlayerCharacter::AMS_PlayerCharacter()
//{
//	PrimaryActorTick.bCanEverTick = false;
//	PrimaryActorTick.bStartWithTickEnabled = false;
//	GetMesh()->bReceivesDecals = false;
//
//	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
//
//	bUseControllerRotationPitch = false;
//	bUseControllerRotationYaw = false;
//	bUseControllerRotationRoll = false;
//
//	GetCharacterMovement()->bOrientRotationToMovement = true;
//	GetCharacterMovement()->bUseControllerDesiredRotation = true;
//	GetCharacterMovement()->RotationRate = FRotator(0.0f, 600.0f, 0.0f);
//	GetCharacterMovement()->AirControl = 0.35f;
//	GetCharacterMovement()->GroundFriction = 4.f;
//	GetCharacterMovement()->MaxWalkSpeed = 400.f;
//	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
//	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
//
//	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
//	SpringArm->SetupAttachment(RootComponent);
//	SpringArm->SetRelativeRotation(FRotator(-40.f, 0.f, 0.f));
//	SpringArm->TargetArmLength = 1000.f;
//	SpringArm->bUsePawnControlRotation = true;
//	SpringArm->bInheritPitch = false;
//	SpringArm->bInheritRoll = true;
//	SpringArm->bInheritYaw = true;
//	SpringArm->bDoCollisionTest = true;
//
//	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
//	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
//	Camera->bUsePawnControlRotation = false;
//
//	// Ability System
//	AbilitySystemComponent = CreateDefaultSubobject<UMS_PlayerAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
//	AbilitySystemComponent->SetIsReplicated(true);
//	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
//
//	AttributeSet = CreateDefaultSubobject<UMS_PlayerAttributeSet>(TEXT("AttributeSet"));
//}
//
//void AMS_PlayerCharacter::PostInitializeComponents()
//{
//	Super::PostInitializeComponents();
//}
//
//void AMS_PlayerCharacter::PostLoad()
//{
//	Super::PostLoad();
//
//	if (CharacterStartUpData.IsValid() && !CharacterStartUpData.IsNull())
//	{
//		// 기본 시작 데이터 동기 로딩
//		if (UDA_PlayerStartUpData* StartUpData = Cast<UDA_PlayerStartUpData>(CharacterStartUpData.LoadSynchronous()))
//		{
//			// 기본 어빌리티 부여 및 게임플레이 이펙트 적용
//			StartUpData->GiveToAbilitySystemComponent(AbilitySystemComponent);
//		}
//	}
//}
//
//void AMS_PlayerCharacter::OnRep_PlayerState()
//{
//	Super::OnRep_PlayerState();
//
//	// 클라이언트도 어빌리티 시스템 컴포넌트 초기화 및 기본 시작 게임플레이 적용
//	AbilitySystemComponent->InitAbilityActorInfo(this, this);
//	if (CharacterStartUpData.IsValid() && !CharacterStartUpData.IsNull())
//	{
//		// 기본 시작 데이터 동기 로딩
//		if (UDA_PlayerStartUpData* StartUpData = Cast<UDA_PlayerStartUpData>(CharacterStartUpData.LoadSynchronous()))
//		{
//			// 기본 어빌리티 부여 및 게임플레이 이펙트 적용
//			StartUpData->GiveToAbilitySystemComponent(AbilitySystemComponent);
//		}
//	}
//}
//
//UAbilitySystemComponent* AMS_PlayerCharacter::GetAbilitySystemComponent() const
//{
//	if (AbilitySystemComponent)
//	{
//		return AbilitySystemComponent;
//	}
//	return nullptr;
//}
//
//UMS_PlayerAttributeSet* AMS_PlayerCharacter::GetAttributeSet() const
//{
//	if (AttributeSet)
//	{
//		return AttributeSet;
//	}
//	return nullptr;
//}
//
//void AMS_PlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
//{
//	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//}
//
//void AMS_PlayerCharacter::BeginPlay()
//{
//	Super::BeginPlay();
//
//}
//
//void AMS_PlayerCharacter::PossessedBy(AController* NewController)
//{
//	Super::PossessedBy(NewController);
//}
//
//void AMS_PlayerCharacter::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//}
//
//void AMS_PlayerCharacter::PawnClientRestart()
//{
//	Super::PawnClientRestart();
//
//	if (APlayerController* PC = Cast<APlayerController>(GetController()))
//	{
//		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
//		{
//			Subsystem->ClearAllMappings();
//			Subsystem->AddMappingContext(DefaultMappingContext, 0);
//		}
//	}
//}
//
//void AMS_PlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
//{
//	Super::SetupPlayerInputComponent(PlayerInputComponent);
//
//}
