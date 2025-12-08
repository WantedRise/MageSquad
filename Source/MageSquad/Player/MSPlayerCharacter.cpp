// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MSPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"

#include "AbilitySystem/ASC/MSPlayerAbilitySystemComponent.h"
#include "AbilitySystem/AttributeSets/MSPlayerAttributeSet.h"

#include "DataAssets/Player/DA_PlayerStartUpData.h"

#include "Net/UnrealNetwork.h"

AMSPlayerCharacter::AMSPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	GetMesh()->bReceivesDecals = false;

	// 네트워크 설정
	bReplicates = true;
	bAlwaysRelevant = true;
	SetNetUpdateFrequency(30.f);
	SetMinNetUpdateFrequency(5.f);

	// 캐릭터 & 카메라 설정
	GetCapsuleComponent()->InitCapsuleSize(50.f, 100.f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 600.0f, 0.0f);
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->GroundFriction = 4.f;
	GetCharacterMovement()->MaxWalkSpeed = 400.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->SetRelativeRotation(FRotator(-40.f, 0.f, 0.f));
	SpringArm->TargetArmLength = 1000.f;
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = true;
	SpringArm->bInheritYaw = true;
	SpringArm->bDoCollisionTest = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	AbilitySystemComponent = CreateDefaultSubobject<UMSPlayerAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

	// Minimal 모드는 클라이언트에는 필요한 정보만 복제하여 네트워크 부하를 줄임
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<UMSPlayerAttributeSet>(TEXT("AttributeSet"));

	// 자동 공격 설정
	AutoAttackCooldown = 0.5f;
	TimeSinceLastAttack = 0.0f;
	bAutoAttacking = true;
}

void AMSPlayerCharacter::PostLoad()
{
	Super::PostLoad();

	// 에디터 / 런타임 로드 시, 기본 시작 데이터를 세팅
	if (PlayerStartUpDataAsset)
	{
		SetPlayerStartAbilityData(PlayerStartUpDataAsset->PlayerStartAbilityData);
	}
}

void AMSPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 런타임 중이라도, 시작 데이터 에셋이 존재하지만 아직 세팅이 되지 않았다면, 한번 더 세팅
	if (PlayerStartUpDataAsset && !PlayerStartAbilityData.Abilties.Num())
	{
		SetPlayerStartAbilityData(PlayerStartUpDataAsset->PlayerStartAbilityData);
	}
}

void AMSPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}

	// 시작 데이터 기반으로 어빌리티/이펙트 부여 (서버 전용)
	GivePlayerStartAbilities();
	ApplyPlayerStartEffects();
}

void AMSPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 플레이어 시작 데이터 복제
	DOREPLIFETIME(AMSPlayerCharacter, PlayerStartAbilityData);
}

void AMSPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// 클라이언트 ASC 초기화
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}

	// 시작 데이터 기반으로 어빌리티/이펙트 부여는 서버 전용이므로 호출 안 함
	// 다만, HUD 초기화 등은 가능
}

void AMSPlayerCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings();
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AMSPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Move
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMSPlayerCharacter::Move);
	}
}

void AMSPlayerCharacter::Move(const FInputActionValue& Value)
{
	// 2D 축 입력을 FVector2D로 변환
	const FVector2D MoveValue = Value.Get<FVector2D>();

	// 입력/컨트롤러가 유효하지 않거나, 입력 값이 거의 0이면 무시
	if (!Controller || MoveValue.IsNearlyZero())
	{
		return;
	}

	// 컨트롤러의 회전값에서 Yaw(좌우)만 사용해서 월드 방향 벡터 생성
	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);

	// 전진(Forward) 방향(X축), 오른쪽(Right) 방향(Y축) 구하기
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	// InputValue.Y : 전/후 (W,S)
	// InputValue.X : 좌/우 (A,D)
	if (!FMath::IsNearlyZero(MoveValue.Y))
	{
		AddMovementInput(ForwardDirection, MoveValue.Y);
	}

	if (!FMath::IsNearlyZero(MoveValue.X))
	{
		AddMovementInput(RightDirection, MoveValue.X);
	}
}

void AMSPlayerCharacter::AutoAttack()
{
}

void AMSPlayerCharacter::SetPlayerStartAbilityData(const FStartAbilityData& InPlayerStartData)
{
	PlayerStartAbilityData = InPlayerStartData;
	InitFromPlayerStartAbilityData(PlayerStartAbilityData, false);
}

void AMSPlayerCharacter::OnRep_PlayerStartAbilityData()
{
	// 클라이언트에서 복제된 시작 데이터 반영
	InitFromPlayerStartAbilityData(PlayerStartAbilityData, true);
}

void AMSPlayerCharacter::InitFromPlayerStartAbilityData(const FStartAbilityData& InPlayerStartData, bool bFromReplication)
{

}

UAbilitySystemComponent* AMSPlayerCharacter::GetAbilitySystemComponent() const
{
	if (AbilitySystemComponent)
	{
		return AbilitySystemComponent;
	}
	return nullptr;
}

bool AMSPlayerCharacter::ApplyGameplayEffectToSelf(TSubclassOf<class UGameplayEffect> Effect, FGameplayEffectContextHandle InEffectContextHandle)
{
	// 유효한 이펙트 클래스와 ASC가 필요한 서버 전용 헬퍼 함수
	if (!Effect.Get() || !AbilitySystemComponent)
	{
		return false;
	}

	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(Effect, /*Level*/ 1.0f, InEffectContextHandle);
	if (SpecHandle.IsValid())
	{
		FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		return ActiveGEHandle.WasSuccessfullyApplied();
	}

	return false;
}

void AMSPlayerCharacter::GivePlayerStartAbilities()
{
	// 서버에서만 어빌리티를 부여
	if (!HasAuthority() || !AbilitySystemComponent)
	{
		return;
	}

	// 플레이어 시작 데이터의 어빌리티 배열을 순회하며 부여
	for (TSubclassOf<UGameplayAbility> DefaultAbilityClass : PlayerStartAbilityData.Abilties)
	{
		if (*DefaultAbilityClass)
		{
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(DefaultAbilityClass));
		}
	}
}

void AMSPlayerCharacter::ApplyPlayerStartEffects()
{
	// 서버에서만 GameplayEffect를 적용
	if (!HasAuthority() || !AbilitySystemComponent)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	// 플레이어 시작 데이터의 이펙트 배열을 순회하며 적용
	for (TSubclassOf<UGameplayEffect> DefaultEffectClass : PlayerStartAbilityData.Effects)
	{
		if (*DefaultEffectClass)
		{
			ApplyGameplayEffectToSelf(DefaultEffectClass, EffectContext);
		}
	}
}
