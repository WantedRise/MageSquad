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
#include "MSGameplayTags.h"

#include "DataAssets/Player/DA_PlayerStartUpData.h"

#include "Net/UnrealNetwork.h"

AMSPlayerCharacter::AMSPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->bReceivesDecals = false;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Enemy ���� �ݸ������� ����
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("MSPlayer"));

	// ��Ʈ��ũ ����
	bReplicates = true;
	bAlwaysRelevant = true;
	SetNetUpdateFrequency(30.f); // �⺻������ ���缭 �뿪�� ����
	SetMinNetUpdateFrequency(5.f);

	// ĳ���� & ī�޶� ����
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

	// ���� �±� ����
	Tags.AddUnique(TEXT("Player"));
}

void AMSPlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// �⺻ ���� �����͸� ����
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

	// ���ÿ��� ����Ǵ� ���� ���� (���� ���� �ʿ� ����)
	if (!IsLocallyControlled())
	{
		return;
	}

	// ī�޶� �� ��/�ƿ� ���� ����
	UpdateCameraZoom(DeltaSecond);
}

void AMSPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// ���������� ����
	if (!HasAuthority()) return;

	// PlayerState ��������
	AMSPlayerState* PS = GetPlayerState<AMSPlayerState>();
	if (!PS) return;

	// ASC, AttributeSet �ʱ�ȭ
	AbilitySystemComponent = Cast<UMSPlayerAbilitySystemComponent>(PS->GetAbilitySystemComponent());
	AttributeSet = PS->GetAttributeSet();

	if (AbilitySystemComponent)
	{
		// GAS�� ���� ������ �����ڴ� PlayerState��, �ƹ�Ÿ�� �ڽ����� �ʱ�ȭ
		AbilitySystemComponent->InitAbilityActorInfo(PS, this);
	}

	// ���� �����Ƽ/����Ʈ �ο� (���� ����)
	GivePlayerStartAbilities();
	ApplyPlayerStartEffects();
	
	// State.Invincible 태그 변경 감지
	if (AbilitySystemComponent != nullptr)
	{
		AbilitySystemComponent->RegisterGameplayTagEvent(
			MSGameplayTags::Player_State_Invincible,
			EGameplayTagEventType::NewOrRemoved
		).AddUObject(this, &AMSPlayerCharacter::OnInvincibilityChanged);
	}

	// HUD ������ ������Ʈ�� �����Ƽ �ý��� ���ε�
	if (HUDDataComponent && AbilitySystemComponent)
	{
		HUDDataComponent->BindToASC_Server(AbilitySystemComponent);
	}

	// ���� �ڵ� ���� ����
	if (bAutoAttackEnabledOnSpawn)
	{
		SetAutoAttackEnabledInternal(true);
	}

	// HUD ���� ������ �ʱ�ȭ
	InitPublicHUDData_Server();
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

	// Ŭ���̾�Ʈ�� �����Ƽ �ý��� ������Ʈ �ʱ�ȭ �� �⺻ ���� ����Ʈ ����

	// PlayerState ��������
	AMSPlayerState* PS = GetPlayerState<AMSPlayerState>();
	if (!PS) return;

	// Ŭ���̾�Ʈ ASC, AttributeSet �ʱ�ȭ
	AbilitySystemComponent = Cast<UMSPlayerAbilitySystemComponent>(PS->GetAbilitySystemComponent());
	AttributeSet = PS->GetAttributeSet();

	if (AbilitySystemComponent)
	{
		// GAS�� ���� ������ �����ڴ� PlayerState��, �ƹ�Ÿ�� �ڽ����� �ʱ�ȭ
		AbilitySystemComponent->InitAbilityActorInfo(PS, this);
	}

	//GivePlayerStartAbilities();
	//ApplyPlayerStartEffects();
}

void AMSPlayerCharacter::UpdateCameraZoom(float DeltaTime)
{
	if (!SpringArm) return;

	// ��ǥ �� ���̱��� �ε巴�� ����
	const float CurrentLength = SpringArm->TargetArmLength;
	const float NewLength = FMath::FInterpTo(CurrentLength, TargetArmLength, DeltaTime, CameraZoomInterpSpeed);
	SpringArm->TargetArmLength = NewLength;
}

void AMSPlayerCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
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
		// �̵� �Է� ����
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMSPlayerCharacter::Move);

		// ī�޶� �� ��/�ƿ� �Է� ����
		EnhancedInputComponent->BindAction(CameraZoomAction, ETriggerEvent::Triggered, this,
		                                   &AMSPlayerCharacter::CameraZoom);

		// ī�޶� �� ��/�ƿ� �Է� ����
		EnhancedInputComponent->BindAction(BlinkAction, ETriggerEvent::Triggered, this, &AMSPlayerCharacter::UseBlink);

		// ��Ŭ�� ���� �Է� ����
		EnhancedInputComponent->BindAction(LeftSkillAction, ETriggerEvent::Started, this,
		                                   &AMSPlayerCharacter::UseLeftSkill);

		// ��Ŭ�� ���� �Է� ����
		EnhancedInputComponent->BindAction(RightSkillAction, ETriggerEvent::Started, this,
		                                   &AMSPlayerCharacter::UseRightSkill);


		// TEST: HP ����/���� �Է� ����
		EnhancedInputComponent->BindAction(TEST_HpIncreaseAction, ETriggerEvent::Started, this,
		                                   &AMSPlayerCharacter::TEST_HpIncrease);
		EnhancedInputComponent->BindAction(TEST_HpDecreaseAction, ETriggerEvent::Started, this,
		                                   &AMSPlayerCharacter::TEST_HpDecrease);
		EnhancedInputComponent->BindAction(TEST_MaxHpIncreaseAction, ETriggerEvent::Started, this,
		                                   &AMSPlayerCharacter::TEST_MaxHpIncrease);
		EnhancedInputComponent->BindAction(TEST_MaxHpDecreaseAction, ETriggerEvent::Started, this,
		                                   &AMSPlayerCharacter::TEST_MaxHpDecrease);
	}
}

void AMSPlayerCharacter::Move(const FInputActionValue& Value)
{
	// 2D �� �Է��� FVector2D�� ��ȯ
	const FVector2D MoveValue = Value.Get<FVector2D>();

	// �Է�/��Ʈ�ѷ��� ��ȿ���� �ʰų�, �Է� ���� ���� 0�̸� ����
	if (!Controller || MoveValue.IsNearlyZero())
	{
		return;
	}

	// ��Ʈ�ѷ��� ȸ�������� Yaw(�¿�)�� ����ؼ� ���� ���� ���� ����
	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);

	// ����(Forward) ����(X��), ������(Right) ����(Y��) ���ϱ�
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	// InputValue.Y : ��/�� (W,S)
	// InputValue.X : ��/�� (A,D)
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

	// ���� �ʹ� ������ �н�
	if (FMath::IsNearlyZero(AxisValue))
	{
		return;
	}

	// ���� ��ũ��(+): ī�޶� ĳ���Ϳ� ������
	// �Ʒ��� ��ũ��(-): ī�޶� ĳ���Ϳ� �ָ�
	TargetArmLength -= AxisValue * CameraZoomStep;

	// �ִ�/�ּ� ī�޶� �� ���̷� Clamp
	TargetArmLength = FMath::Clamp(TargetArmLength, MinCameraZoomLength, MaxCameraZoomLength);
}

void AMSPlayerCharacter::UseBlink(const FInputActionValue& Value)
{
	// ���� ���� �ƴ� ��� ����
	if (!IsLocallyControlled()) return;

	// Ʈ���� �Լ� ȣ��
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
	// ������ �ƴϸ� ����
	if (!HasAuthority()) return;

	// ���� ������ ��� ����
	if (bAutoAttackEnabled == bEnabled) return;

	bAutoAttackEnabled = bEnabled;

	GetWorldTimerManager().ClearTimer(AutoAttackTimerHandle);

	if (bAutoAttackEnabled)
	{
		// �������� �⺻ ������ ����
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
	// ������ �ƴϰų�, �ڵ� ������ ��Ȱ��ȭ ���¶�� ����
	if (!HasAuthority() || !bAutoAttackEnabled) return;

	// �̺�Ʈ �±װ� ��ȿ���� ������ ����
	if (!AttackStartedEventTag.IsValid()) return;

	// �̺�Ʈ �±� ����
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
	// ��ȿ�� �˻�
	if (!IsAllowedAbilityEventTag(EventTag)) return;

	// ���� ����
	if (HasAuthority())
	{
		// �̺�Ʈ �±� ����
		FGameplayEventData Payload;
		Payload.EventTag = EventTag;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, EventTag, Payload);
	}
	// Ŭ���̾�Ʈ ����
	else
	{
		// �������� Ʈ���� ��û
		ServerRPCTriggerAbilityEvent(EventTag);
	}
}

void AMSPlayerCharacter::ServerRPCTriggerAbilityEvent_Implementation(FGameplayTag EventTag)
{
	// ��ȿ�� �˻�
	if (!IsAllowedAbilityEventTag(EventTag)) return;

	// �̺�Ʈ �±� ����
	FGameplayEventData Payload;
	Payload.EventTag = EventTag;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, EventTag, Payload);
}

bool AMSPlayerCharacter::IsAllowedAbilityEventTag(const FGameplayTag& EventTag) const
{
	// ���� �÷��̾ �� �̺�Ʈ �±׸� ������ �ִ��� �˻� (���� �߰�)
	return EventTag.IsValid() /*&& (EventTag == BlinkEventTag || EventTag == AttackStartedEventTag)*/;
}

void AMSPlayerCharacter::SetPlayerData(const FPlayerStartAbilityData& InPlayerData)
{
	PlayerData = InPlayerData;
}

UAbilitySystemComponent* AMSPlayerCharacter::GetAbilitySystemComponent() const
{
	if (AbilitySystemComponent)
	{
		return AbilitySystemComponent;
	}
	return nullptr;
}

bool AMSPlayerCharacter::ApplyGameplayEffectToSelf(TSubclassOf<UGameplayEffect> Effect,
                                                   FGameplayEffectContextHandle InEffectContextHandle)
{
	// ��ȿ�� ����Ʈ Ŭ������ ASC�� �ʿ��� ���� ���� ���� �Լ�
	if (!Effect.Get() || !AbilitySystemComponent) return false;

	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
		Effect, /*Level*/ 1.0f, InEffectContextHandle);
	if (SpecHandle.IsValid())
	{
		FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(
			*SpecHandle.Data.Get());
		return ActiveGEHandle.WasSuccessfullyApplied();
	}

	return false;
}

void AMSPlayerCharacter::GivePlayerStartAbilities()
{
	// ���������� �����Ƽ�� �ο�
	if (!HasAuthority() || !AbilitySystemComponent) return;

	// �÷��̾� ���� �������� �����Ƽ �迭�� ��ȸ�ϸ� �ο�
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
	// ���������� GameplayEffect�� ����
	if (!HasAuthority() || !AbilitySystemComponent) return;

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	// �÷��̾� ���� �������� ����Ʈ �迭�� ��ȸ�ϸ� ����
	for (TSubclassOf<UGameplayEffect> DefaultEffectClass : PlayerData.Effects)
	{
		if (*DefaultEffectClass)
		{
			ApplyGameplayEffectToSelf(DefaultEffectClass, EffectContext);
		}
	}
}

void AMSPlayerCharacter::InitPublicHUDData_Server()
{
	if (!HasAuthority() || !HUDDataComponent) return;

	// �÷��̾� �̸� �� ������ �ʱ�ȭ
	if (APlayerState* PS = GetPlayerState())
	{
		HUDDataComponent->BindDisplayName_Server(FText::FromString(PS->GetPlayerName()));
	}
	HUDDataComponent->BindPortraitIcon_Server(PortraitIcon);
}

void AMSPlayerCharacter::OnInvincibilityChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bool bIsInvincible = (NewCount > 0);

	UE_LOG(LogTemp, Warning, TEXT("Invincibility changed: %s"),
	       bIsInvincible ? TEXT("ON") : TEXT("OFF"));

	SetInvincibleCollision(bIsInvincible);
}

void AMSPlayerCharacter::SetInvincibleCollision(bool bInvincible)
{
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (!Capsule)
	{
		return;
	}

	if (bInvincible)
	{
		// 무적: MSEnemy와 Ignore
		// ECC_GameTraceChannel3 = MSEnemy
		Capsule->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Ignore);
		UE_LOG(LogTemp, Log, TEXT("Collision Profile: PlayerInvincible (Ignore MSEnemy)"));
	}
	else
	{
		// 일반: MSEnemy와 Overlap
		Capsule->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap);
		UE_LOG(LogTemp, Log, TEXT("Collision Profile: Player (Overlap MSEnemy)"));
	}
}

void AMSPlayerCharacter::TEST_HpIncrease(const FInputActionValue& Value)
{
	if (!IsLocallyControlled()) return;
	TriggerAbilityEvent(TEST_HpIncreaseEventTag);
}

void AMSPlayerCharacter::TEST_HpDecrease(const FInputActionValue& Value)
{
	if (!IsLocallyControlled()) return;
	TriggerAbilityEvent(TEST_HpDecreaseEventTag);
}

void AMSPlayerCharacter::TEST_MaxHpIncrease(const FInputActionValue& Value)
{
	if (!IsLocallyControlled()) return;
	TriggerAbilityEvent(TEST_MaxHpIncreaseEventTag);
}

void AMSPlayerCharacter::TEST_MaxHpDecrease(const FInputActionValue& Value)
{
	if (!IsLocallyControlled()) return;
	TriggerAbilityEvent(TEST_MaxHpDecreaseEventTag);
}
