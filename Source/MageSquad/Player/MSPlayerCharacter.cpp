// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MSPlayerCharacter.h"
#include "Player/MSPlayerState.h"

#include "Components/Player/MSHUDDataComponent.h"
#include "Engine/Texture2D.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"

#include "AbilitySystem/ASC/MSPlayerAbilitySystemComponent.h"
#include "AbilitySystem/AttributeSets/MSPlayerAttributeSet.h"
#include "AbilitySystemBlueprintLibrary.h"

#include "DataAssets/Player/DA_PlayerStartUpData.h"

#include "Net/UnrealNetwork.h"

AMSPlayerCharacter::AMSPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->bReceivesDecals = false;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Enemy 전용 콜리전으로 설정
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("MSPlayer"));

	// 네트워크 설정
	bReplicates = true;
	bAlwaysRelevant = true;
	SetNetUpdateFrequency(30.f); // 기본값보다 낮춰서 대역폭 절약
	SetMinNetUpdateFrequency(5.f);

	// 캐릭터 & 카메라 설정
	GetCapsuleComponent()->InitCapsuleSize(50.f, 100.f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 400.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = 500.f;

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

	StaffMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Staff"));
	StaffMesh->SetupAttachment(GetMesh(), StaffAttachSocketName);
	StaffMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StaffMesh->SetGenerateOverlapEvents(false);
	StaffMesh->PrimaryComponentTick.bCanEverTick = false;
	StaffMesh->PrimaryComponentTick.bStartWithTickEnabled = false;
	StaffMesh->bReceivesDecals = false;

	HUDDataComponent = CreateDefaultSubobject<UMSHUDDataComponent>(TEXT("HUDDataComponent"));

	// 액터 태그 설정
	Tags.AddUnique(TEXT("Player"));
}

void AMSPlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// 기본 시작 데이터를 세팅
	if (::IsValid(PlayerStartUpData))
	{
		SetPlayerData(PlayerStartUpData->PlayerStartAbilityData);
	}
}

void AMSPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AMSPlayerCharacter::Tick(float DeltaSecond)
{
	Super::Tick(DeltaSecond);

	// 로컬에서 제어되는 폰만 수행 (서버 연동 필요 없음)
	if (!IsLocallyControlled())
	{
		return;
	}

	// 카메라 줌 인/아웃 보간 수행
	UpdateCameraZoom(DeltaSecond);
}

void AMSPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// PlayerState 가져오기
	AMSPlayerState* PS = GetPlayerState<AMSPlayerState>();
	if (!PS) return;

	// ASC, AttributeSet 초기화
	AbilitySystemComponent = Cast<UMSPlayerAbilitySystemComponent>(PS->GetAbilitySystemComponent());
	AttributeSet = PS->GetAttributeSet();

	if (AbilitySystemComponent)
	{
		// GAS의 액터 정보를 소유자는 PlayerState로, 아바타는 자신으로 초기화
		AbilitySystemComponent->InitAbilityActorInfo(PS, this);
	}

	// 시작 어빌리티/이펙트 부여 (서버 전용)
	GivePlayerStartAbilities();
	ApplyPlayerStartEffects();

	// HUD 리플리케이션 (서버 -> 모든 클라이언트)
	if (HasAuthority() && HUDDataComponent && AbilitySystemComponent)
	{
		// 어빌리티 시스템, 플레이어 아이콘 동기화
		HUDDataComponent->BindToASC_Server(AbilitySystemComponent);
		HUDDataComponent->SetPortraitIcon_Server(PortraitIcon);
	}

	// 서버 자동 공격 시작
	if (HasAuthority() && bAutoAttackEnabledOnSpawn)
	{
		SetAutoAttackEnabledInternal(true);
	}
}

void AMSPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMSPlayerCharacter, PlayerData);
	DOREPLIFETIME(AMSPlayerCharacter, bAutoAttackEnabled);
}

void AMSPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// 클라이언트도 어빌리티 시스템 컴포넌트 초기화 및 기본 적용 이펙트 적용

	// PlayerState 가져오기
	AMSPlayerState* PS = GetPlayerState<AMSPlayerState>();
	if (!PS) return;

	// 클라이언트 ASC, AttributeSet 초기화
	AbilitySystemComponent = Cast<UMSPlayerAbilitySystemComponent>(PS->GetAbilitySystemComponent());
	AttributeSet = PS->GetAttributeSet();

	if (AbilitySystemComponent)
	{
		// GAS의 액터 정보를 소유자는 PlayerState로, 아바타는 자신으로 초기화
		AbilitySystemComponent->InitAbilityActorInfo(PS, this);
	}

	GivePlayerStartAbilities();
	ApplyPlayerStartEffects();
}

void AMSPlayerCharacter::UpdateCameraZoom(float DeltaTime)
{
	if (!SpringArm) return;

	// 목표 줌 길이까지 부드럽게 보간
	const float CurrentLength = SpringArm->TargetArmLength;
	const float NewLength = FMath::FInterpTo(CurrentLength, TargetArmLength, DeltaTime, CameraZoomInterpSpeed);
	SpringArm->TargetArmLength = NewLength;
}

void AMSPlayerCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings();
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}

void AMSPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// 이동 입력 맵핑
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMSPlayerCharacter::Move);

		// 카메라 줌 인/아웃 입력 맵핑
		EnhancedInputComponent->BindAction(CameraZoomAction, ETriggerEvent::Triggered, this, &AMSPlayerCharacter::CameraZoom);

		// 카메라 줌 인/아웃 입력 맵핑
		EnhancedInputComponent->BindAction(BlinkAction, ETriggerEvent::Triggered, this, &AMSPlayerCharacter::UseBlink);

		// 좌클릭 공격 입력 맵핑
		EnhancedInputComponent->BindAction(LeftSkillAction, ETriggerEvent::Triggered, this, &AMSPlayerCharacter::UseLeftSkill);

		// 우클릭 공격 입력 맵핑
		EnhancedInputComponent->BindAction(RightSkillAction, ETriggerEvent::Triggered, this, &AMSPlayerCharacter::UseRightSkill);
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

void AMSPlayerCharacter::CameraZoom(const FInputActionValue& Value)
{
	const float AxisValue = Value.Get<float>();

	// 값이 너무 작으면 패스
	if (FMath::IsNearlyZero(AxisValue))
	{
		return;
	}

	// 위로 스크롤(+): 카메라를 캐릭터와 가까이
	// 아래로 스크롤(-): 카메라를 캐릭터와 멀리
	TargetArmLength -= AxisValue * CameraZoomStep;

	// 최대/최소 카메라 줌 길이로 Clamp
	TargetArmLength = FMath::Clamp(TargetArmLength, MinCameraZoomLength, MaxCameraZoomLength);
}

void AMSPlayerCharacter::UseBlink(const FInputActionValue& Value)
{
	// 로컬 폰이 아닌 경우 종료
	if (!IsLocallyControlled()) return;

	// 트리거 함수 호출
	TriggerAbilityEvent(BlinkEventTag);
}

void AMSPlayerCharacter::UseLeftSkill(const FInputActionValue& Value)
{

}

void AMSPlayerCharacter::UseRightSkill(const FInputActionValue& Value)
{

}

void AMSPlayerCharacter::StartAutoAttack()
{
	if (HasAuthority())
	{
		SetAutoAttackEnabledInternal(true);
	}
	else if (IsLocallyControlled())
	{
		ServerRPCSetAutoAttackEnabled(true);
	}
}

void AMSPlayerCharacter::StopAutoAttack()
{
	if (HasAuthority())
	{
		SetAutoAttackEnabledInternal(false);
	}
	else if (IsLocallyControlled())
	{
		ServerRPCSetAutoAttackEnabled(false);
	}
}

void AMSPlayerCharacter::SetAutoAttackEnabledInternal(bool bEnabled)
{
	// 서버가 아니면 종료
	if (!HasAuthority()) return;

	// 같은 설정일 경우 무시
	if (bAutoAttackEnabled == bEnabled) return;

	bAutoAttackEnabled = bEnabled;

	GetWorldTimerManager().ClearTimer(AutoAttackTimerHandle);

	if (bAutoAttackEnabled)
	{
		// 서버에서 기본 공격을 수행
		GetWorldTimerManager().SetTimer(
			AutoAttackTimerHandle,
			this,
			&AMSPlayerCharacter::HandleAutoAttack_Server,
			AutoAttackInterval,
			true,
			0.5f
		);
	}
}

void AMSPlayerCharacter::HandleAutoAttack_Server()
{
	// 서버가 아니거나, 자동 공격이 비활성화 상태라면 종료
	if (!HasAuthority() || !bAutoAttackEnabled) return;

	// 이벤트 태그가 유효하지 않으면 종료
	if (!AttackStartedEventTag.IsValid()) return;

	// 이벤트 태그 전달
	FGameplayEventData Payload;
	Payload.EventTag = AttackStartedEventTag;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, AttackStartedEventTag, Payload);
}

void AMSPlayerCharacter::ServerRPCSetAutoAttackEnabled_Implementation(bool bEnabled)
{
	SetAutoAttackEnabledInternal(bEnabled);
}

void AMSPlayerCharacter::TriggerAbilityEvent(const FGameplayTag& EventTag)
{
	// 유효성 검사
	if (!IsAllowedAbilityEventTag(EventTag)) return;

	// 서버 로직
	if (HasAuthority())
	{
		// 이벤트 태그 전달
		FGameplayEventData Payload;
		Payload.EventTag = EventTag;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, EventTag, Payload);
	}
	// 클라이언트 로직
	else
	{
		// 서버에게 트리거 요청
		ServerRPCTriggerAbilityEvent(EventTag);
	}
}

void AMSPlayerCharacter::ServerRPCTriggerAbilityEvent_Implementation(FGameplayTag EventTag)
{
	// 유효성 검사
	if (!IsAllowedAbilityEventTag(EventTag)) return;

	// 이벤트 태그 전달
	FGameplayEventData Payload;
	Payload.EventTag = EventTag;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, EventTag, Payload);
}

bool AMSPlayerCharacter::IsAllowedAbilityEventTag(const FGameplayTag& EventTag) const
{
	// 로컬 플레이어가 이 이벤트 태그를 가지고 있는지 검사 (이후 추가)
	return EventTag.IsValid() && (EventTag == BlinkEventTag || EventTag == AttackStartedEventTag);
}

void AMSPlayerCharacter::SetPlayerData(const FPlayerStartAbilityData& InPlayerData)
{
	PlayerData = InPlayerData;
}

UAbilitySystemComponent* AMSPlayerCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

bool AMSPlayerCharacter::ApplyGameplayEffectToSelf(TSubclassOf<UGameplayEffect> Effect, FGameplayEffectContextHandle InEffectContextHandle)
{
	// 유효한 이펙트 클래스와 ASC가 필요한 서버 전용 헬퍼 함수
	if (!Effect.Get() || !AbilitySystemComponent) return false;

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
	if (!HasAuthority() || !AbilitySystemComponent) return;

	// 플레이어 시작 데이터의 어빌리티 배열을 순회하며 부여
	for (TSubclassOf<UGameplayAbility> DefaultAbilityClass : PlayerData.Abilties)
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
	if (!HasAuthority() || !AbilitySystemComponent) return;

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	// 플레이어 시작 데이터의 이펙트 배열을 순회하며 적용
	for (TSubclassOf<UGameplayEffect> DefaultEffectClass : PlayerData.Effects)
	{
		if (*DefaultEffectClass)
		{
			ApplyGameplayEffectToSelf(DefaultEffectClass, EffectContext);
		}
	}
}
