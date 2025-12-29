// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MSPlayerCharacter.h"
#include "Player/MSPlayerState.h"
#include "Player/MSPlayerController.h"

#include "Components/Player/MSHUDDataComponent.h"
#include "Engine/Texture2D.h"

#include "Camera/CameraComponent.h"
#include "Camera/CameraShakeBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"

#include "AbilitySystem/ASC/MSPlayerAbilitySystemComponent.h"
#include "AbilitySystem/AttributeSets/MSPlayerAttributeSet.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "MSGameplayTags.h"

#include "Actors/Experience/MSExperienceOrb.h"
#include "Actors/Revival/MSTeamReviveActor.h"

#include "Widgets/HUD/MSOverheadNameWidget.h"

#include "DataAssets/Player/DA_PlayerStartUpData.h"

#include "GameStates/MSGameState.h"

#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"

AMSPlayerCharacter::AMSPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->bReceivesDecals = false;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Player 전용 콜리전으로 설정
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("MSPlayer"));
	GetCapsuleComponent()->SetCollisionObjectType(ECC_GameTraceChannel1);

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

	ExperiencePickupCollision = CreateDefaultSubobject<USphereComponent>(TEXT("ExperiencePickupCollision"));
	ExperiencePickupCollision->SetupAttachment(RootComponent);
	ExperiencePickupCollision->InitSphereRadius(BaseExperiencePickupRange);
	ExperiencePickupCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ExperiencePickupCollision->SetGenerateOverlapEvents(false);
	ExperiencePickupCollision->SetCollisionProfileName(TEXT("MSPlayerPickUp"));

	OverheadNameWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadNameWidgetComponent"));
	OverheadNameWidgetComp->SetupAttachment(GetRootComponent());
	OverheadNameWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	OverheadNameWidgetComp->SetDrawAtDesiredSize(true);
	OverheadNameWidgetComp->SetRelativeLocation(FVector(0.f, 0.f, 150.f));
	OverheadNameWidgetComp->SetCollisionProfileName(TEXT("NoCollision"));
	OverheadNameWidgetComp->SetGenerateOverlapEvents(false);
	OverheadNameWidgetComp->SetTwoSided(false);
	OverheadNameWidgetComp->SetHiddenInGame(true);

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

	/*
	* 경험치 픽업 판정 설정
	* 경험치 픽업 판정은 서버에서만 수행
	*/
	if (HasAuthority() && ExperiencePickupCollision)
	{
		ExperiencePickupCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		ExperiencePickupCollision->SetGenerateOverlapEvents(true);
		ExperiencePickupCollision->OnComponentBeginOverlap.AddDynamic(this, &AMSPlayerCharacter::OnExperiencePickupSphereBeginOverlap_Server);

		// 획득 반경 보정 속성 변경 델리게이트 바인딩
		UpdateExperiencePickupRange();
	}
	else if (ExperiencePickupCollision)
	{
		ExperiencePickupCollision->SetGenerateOverlapEvents(false);
		ExperiencePickupCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 머리 위에 띄울 이름 위젯 클래스 설정
	if (OverheadNameWidgetClass)
	{
		OverheadNameWidgetComp->SetWidgetClass(OverheadNameWidgetClass);
	}

	// 머리 위 이름 위젯을 초기화 + 바인딩 설정
	// - 가시화: "자기 자신"만 숨김
	// - 텍스트: HUDDataComponent 복제값(RepDisplayName)에 의해 갱신
	RefreshOverheadVisibility();
	BindOverheadNameToHUDData();
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

	// 서버에서만 수행
	if (!HasAuthority()) return;

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

	// 스킬 슬롯 초기화 및 쿨다운 감소 델리게이트 바인딩
	EnsureSkillSlotArrays();
	BindCooldownReductionDelegate();
	RebuildPassiveSkillTimers_Server();

	/*
	* 시작 어빌리티/이펙트 부여 로직
	*/
	{
		// 초기화 시작 태그
		AbilitySystemComponent->AddLooseGameplayTag(MSGameplayTags::Shared_State_Init);

		GivePlayerStartAbilities_Server();
		ApplyPlayerStartEffects_Server();

		// 초기화 끝 태그
		AbilitySystemComponent->RemoveLooseGameplayTag(MSGameplayTags::Shared_State_Init);
	}

	// State.Invincible 태그 변경 감지
	if (AbilitySystemComponent != nullptr)
	{
		AbilitySystemComponent->RegisterGameplayTagEvent(
			MSGameplayTags::Player_State_Invincible,
			EGameplayTagEventType::NewOrRemoved
		).AddUObject(this, &AMSPlayerCharacter::OnInvincibilityChanged);
	}

	// HUD 데이터 바인딩 및 초기화
	if (HUDDataComponent && AbilitySystemComponent)
	{
		// HUD 데이터에 ASC 바인딩
		HUDDataComponent->BindToASC_Server(AbilitySystemComponent);

		// HUD 공개 데이터 초기화
		InitPublicHUDData_Server();

		// 머리 위 이름 위젯을 초기화 + 바인딩 다시 한 번 보정
		RefreshOverheadVisibility();
		BindOverheadNameToHUDData();
	}
}

void AMSPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMSPlayerCharacter, PlayerData);
	DOREPLIFETIME_CONDITION_NOTIFY(AMSPlayerCharacter, SkillSlots, COND_OwnerOnly, REPNOTIFY_Always);
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

	// 클라이언트에서도 슬롯 배열 크기만 보정(OwnerOnly 복제 수신 전 방어)
	EnsureSkillSlotArrays();

	// 머리 위 이름 위젯을 초기화 + 바인딩 다시 한 번 보정
	RefreshOverheadVisibility();
	BindOverheadNameToHUDData();
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

	// 머리 위 이름 위젯을 초기화 + 바인딩 다시 한 번 보정
	RefreshOverheadVisibility();
	BindOverheadNameToHUDData();
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
		EnhancedInputComponent->BindAction(LeftSkillAction, ETriggerEvent::Started, this, &AMSPlayerCharacter::UseLeftSkill);

		// 우클릭 공격 입력 맵핑
		EnhancedInputComponent->BindAction(RightSkillAction, ETriggerEvent::Started, this, &AMSPlayerCharacter::UseRightSkill);


		// TEST: HP 증가/감소 입력 맵핑
		EnhancedInputComponent->BindAction(TEST_HpIncreaseAction, ETriggerEvent::Started, this, &AMSPlayerCharacter::TEST_HpIncrease);
		EnhancedInputComponent->BindAction(TEST_HpDecreaseAction, ETriggerEvent::Started, this, &AMSPlayerCharacter::TEST_HpDecrease);
		EnhancedInputComponent->BindAction(TEST_MaxHpIncreaseAction, ETriggerEvent::Started, this, &AMSPlayerCharacter::TEST_MaxHpIncrease);
		EnhancedInputComponent->BindAction(TEST_MaxHpDecreaseAction, ETriggerEvent::Started, this, &AMSPlayerCharacter::TEST_MaxHpDecrease);
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
	// 로컬 폰이 아닌 경우 종료
	if (!IsLocallyControlled()) return;

	// 액티브 스킬 사용 요청
	RequestUseActiveSkill(EMSSkillSlotIndex::ActiveLeft);
}

void AMSPlayerCharacter::UseRightSkill(const FInputActionValue& Value)
{
	// 로컬 폰이 아닌 경우 종료
	if (!IsLocallyControlled()) return;

	// 액티브 스킬 사용 요청
	RequestUseActiveSkill(EMSSkillSlotIndex::ActiveRight);
}

void AMSPlayerCharacter::RequestUseActiveSkill(EMSSkillSlotIndex SlotIndex)
{
	// 열거형을 바이트 형태로 변환
	const uint8 Slot = static_cast<uint8>(SlotIndex);

	if (HasAuthority())
	{
		// 서버는 바로 처리
		HandleActiveSkillSlot_Server(static_cast<int32>(Slot));
	}
	else
	{
		// 클라이언트는 서버에게 스킬 사용 요청
		ServerRPCUseActiveSkillSlot(Slot);
	}
}

void AMSPlayerCharacter::AcquireSkill(int32 SkillID)
{
	// 서버에서 호출되는 것이 이상적이나, 호출자가 클라이언트일 수 있으므로 보정
	if (HasAuthority())
	{
		ServerRPCAcquireSkill_Implementation(SkillID);
		return;
	}

	if (IsLocallyControlled())
	{
		ServerRPCAcquireSkill(SkillID);
	}
}

void AMSPlayerCharacter::OnRep_SkillSlots()
{
	// 배열 크기 안전 보정
	EnsureSkillSlotArrays();
}

void AMSPlayerCharacter::EnsureSkillSlotArrays()
{
	constexpr int32 SlotCount = 6;
	constexpr int32 PassiveCount = 4;

	// 스킬 슬롯 개수 초기화
	if (SkillSlots.Num() != SlotCount)
	{
		SkillSlots.SetNum(SlotCount);
	}

	// 서버에서만 설정
	if (HasAuthority())
	{
		// 스킬 슬롯 런타임 데이터 개수 초기화
		if (SkillRuntimeData.Num() != SlotCount)
		{
			SkillRuntimeData.SetNum(SlotCount);
		}

		// 현재 스킬 슬롯 배열을 순회하며 스킬 슬롯 런타임 데이터 생성
		for (int32 i = 0; i < SlotCount; ++i)
		{
			if (!SkillRuntimeData[i])
			{
				SkillRuntimeData[i] = NewObject<UMSSkillSlotRuntimeData>(this);
			}
		}

		// 패시브 스킬 타이머 배열도 개수 초기화
		if (PassiveSkillTimerHandles.Num() != PassiveCount)
		{
			PassiveSkillTimerHandles.SetNum(PassiveCount);
		}
	}
}

void AMSPlayerCharacter::ServerRPCAcquireSkill_Implementation(int32 SkillID)
{
	if (!HasAuthority()) return;

	// 스킬 슬롯 보정
	EnsureSkillSlotArrays();

	AMSPlayerState* PS = GetPlayerState<AMSPlayerState>();
	if (!PS)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Skill] ServerRPCAcquireSkill: PlayerState is null"));
		return;
	}
	
	// 스킬 데이터
	const FMSSkillList* FoundSkill = PS->GetOwnedSkillByID(SkillID);
	if (!FoundSkill)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Skill] ServerRPCAcquireSkill: Owned skill not found. SkillID=%d"), SkillID);
		return;
	}
	FMSSkillList Row = *FoundSkill;

	// 스킬 슬롯에 데이터 할당
	EquipSkillFromRow_Server(Row);

	// 스킬 자동 발동 타이머 설정
	RebuildPassiveSkillTimers_Server();
}

void AMSPlayerCharacter::ServerRPCUseActiveSkillSlot_Implementation(uint8 SlotIndex)
{
	if (!HasAuthority()) return;

	// 스킬 사용
	HandleActiveSkillSlot_Server(static_cast<int32>(SlotIndex));
}

void AMSPlayerCharacter::HandleActiveSkillSlot_Server(int32 SlotIndex)
{
	if (!HasAuthority()) return;

	// 스킬 슬롯 보정
	EnsureSkillSlotArrays();

	// 스킬 슬롯에 없는 스킬인 경우 종료
	if (!SkillSlots.IsValidIndex(SlotIndex) || !SkillSlots[SlotIndex].IsValid()) return;

	// 해당 스킬에 대한 스킬 슬롯 데이터 가져오기
	const FMSPlayerSkillSlotNet& Slot = SkillSlots[SlotIndex];
	if (!Slot.SkillEventTag.IsValid()) return;

	// 게임플레이 이벤트 데이터로 스킬에 전달할 데이터 설정
	FGameplayEventData Payload;
	Payload.EventTag = Slot.SkillEventTag;
	Payload.EventMagnitude = static_cast<float>(Slot.SkillLevel);
	Payload.OptionalObject = SkillRuntimeData.IsValidIndex(SlotIndex) ? SkillRuntimeData[SlotIndex] : nullptr;

	// 스킬 어빌리티 이벤트 트리거
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, Slot.SkillEventTag, Payload);
}

void AMSPlayerCharacter::EquipSkillFromRow_Server(const FMSSkillList& Row)
{
	if (!HasAuthority()) return;

	// DT의 SkillType 규약
	// 1: 자동(패시브), 2: 좌클릭 액티브, 3: 우클릭 액티브
	if (Row.SkillType == 1)
	{
		// 새로 추가할 수 있는 패시브 슬롯 탐색
		const int32 PassiveSlotIndex = FindOrAllocatePassiveSlotIndex_Server(Row.SkillID);

		// 패시브 스킬 슬롯이 모두 찬 경우
		if (PassiveSlotIndex == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Player-Skill] All passive skill slots are full and no more can be added."));
		}
		else
		{
			SetSkillSlot_Server(PassiveSlotIndex, Row);
		}
	}
	else if (Row.SkillType == 2)
	{
		SetSkillSlot_Server(static_cast<int32>(EMSSkillSlotIndex::ActiveLeft), Row);
	}
	else if (Row.SkillType == 3)
	{
		SetSkillSlot_Server(static_cast<int32>(EMSSkillSlotIndex::ActiveRight), Row);
	}
}

void AMSPlayerCharacter::SetSkillSlot_Server(int32 SlotIndex, const FMSSkillList& Row)
{
	if (!HasAuthority()) return;

	// 스킬 슬롯 보정
	EnsureSkillSlotArrays();

	if (!SkillSlots.IsValidIndex(SlotIndex)) return;

	// 스킬 슬롯 정보 초기화
	FMSPlayerSkillSlotNet& Slot = SkillSlots[SlotIndex];
	Slot.SkillType = Row.SkillType;
	Slot.SkillID = Row.SkillID;
	Slot.SkillLevel = Row.SkillLevel;
	Slot.SkillEventTag = Row.SkillEventTag;
	Slot.BaseCoolTime = Row.CoolTime;

	// 스킬 런타임 데이터에 초기화 
	if (SkillRuntimeData.IsValidIndex(SlotIndex) && SkillRuntimeData[SlotIndex])
	{
		SkillRuntimeData[SlotIndex]->InitFromRow(Row, SlotIndex);
	}
}

int32 AMSPlayerCharacter::FindOrAllocatePassiveSlotIndex_Server(int32 SkillID) const
{
	// 패시브 스킬 시작/끝 열거현 번호
	constexpr int32 PassiveStart = static_cast<int32>(EMSSkillSlotIndex::Passive01);
	constexpr int32 PassiveEnd = static_cast<int32>(EMSSkillSlotIndex::Passive04);

	// 이미 같은 스킬을 갖고 있으면 해당 슬롯을 갱신(스킬 레벨업)
	for (int32 i = PassiveStart; i <= PassiveEnd; ++i)
	{
		if (SkillSlots.IsValidIndex(i) && SkillSlots[i].SkillID == SkillID)
		{
			return i;
		}
	}

	// 앞의 빈 슬롯 우선
	for (int32 i = PassiveStart; i <= PassiveEnd; ++i)
	{
		if (SkillSlots.IsValidIndex(i) && SkillSlots[i].SkillID == 0)
		{
			return i;
		}
	}

	// 모두 찼으면 예외 처리 및 로그 출력
	return 0;
}

void AMSPlayerCharacter::RebuildPassiveSkillTimers_Server()
{
	if (!HasAuthority()) return;

	// 스킬 슬롯 보정
	EnsureSkillSlotArrays();

	// 기존 타이머 모두 제거
	for (FTimerHandle& Handle : PassiveSkillTimerHandles)
	{
		GetWorldTimerManager().ClearTimer(Handle);
	}

	// 패시브 스킬 시작 슬롯 인덱스, 패시브 개수
	constexpr int32 PassiveStart = static_cast<int32>(EMSSkillSlotIndex::Passive01);
	constexpr int32 PassiveCount = 4;

	for (int32 i = 0; i < PassiveCount; ++i)
	{
		// 슬롯 인덱스
		const int32 SlotIndex = PassiveStart + i;
		if (!SkillSlots.IsValidIndex(SlotIndex)) continue;

		// 슬롯의 스킬 데이터 가져오기
		const FMSPlayerSkillSlotNet& Slot = SkillSlots[SlotIndex];
		if (!Slot.IsValid()) continue;

		// 패시브(자동)만 타이머 설정
		if (Slot.SkillType != 1) continue;

		// 스킬 주기 계산
		const float Interval = ComputeFinalInterval(Slot.BaseCoolTime);
		if (Interval <= 0.f) continue;

		// 패시브 스킬 자동 발동 타이머 바인딩
		FTimerDelegate Delegate;
		Delegate.BindUObject(this, &AMSPlayerCharacter::HandlePassiveSkillTimer_Server, SlotIndex);

		// 패시브 스킬 자동 발동 타이머 설정
		GetWorldTimerManager().SetTimer(PassiveSkillTimerHandles[i], Delegate, Interval, true, Interval);
	}
}

void AMSPlayerCharacter::HandlePassiveSkillTimer_Server(int32 SlotIndex)
{
	if (!HasAuthority()) return;

	if (!SkillSlots.IsValidIndex(SlotIndex) || !SkillSlots[SlotIndex].IsValid()) return;

	// 슬롯의 스킬 데이터 가져오기
	const FMSPlayerSkillSlotNet& Slot = SkillSlots[SlotIndex];
	if (!Slot.SkillEventTag.IsValid()) return;

	// 게임플레이 이벤트 데이터로 스킬에 전달할 데이터 설정
	FGameplayEventData Payload;
	Payload.EventTag = Slot.SkillEventTag;
	Payload.EventMagnitude = static_cast<float>(Slot.SkillLevel);
	Payload.OptionalObject = SkillRuntimeData.IsValidIndex(SlotIndex) ? SkillRuntimeData[SlotIndex] : nullptr;

	// 스킬 어빌리티 이벤트 트리거
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, Slot.SkillEventTag, Payload);
}

void AMSPlayerCharacter::BindCooldownReductionDelegate()
{
	if (!HasAuthority()) return;

	if (!AbilitySystemComponent || !AttributeSet) return;

	// 중복 바인딩 방지
	if (AttributeSetCooldownReductionChangedHandle.IsValid())
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetCooldownReductionAttribute())
			.Remove(AttributeSetCooldownReductionChangedHandle);
		AttributeSetCooldownReductionChangedHandle.Reset();
	}

	// 플레이어 능력치(AttributeSet)의 쿨타임 감소 속성 변경 델리게이트 바인딩
	AttributeSetCooldownReductionChangedHandle = AbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetCooldownReductionAttribute())
		.AddUObject(this, &AMSPlayerCharacter::OnCooldownReductionChanged);
}

void AMSPlayerCharacter::OnCooldownReductionChanged(const FOnAttributeChangeData& Data)
{
	if (!HasAuthority()) return;

	// 플레이어 능력치(AttributeSet)의 쿨타임 감소 속성이 변경됨에 따라
	// 패시브 스킬 자동 발동 타이머를 재구성
	RebuildPassiveSkillTimers_Server();
}

float AMSPlayerCharacter::ComputeFinalInterval(float BaseCoolTime) const
{
	if (BaseCoolTime <= 0.f) return 0.f;

	// 플레이어 능력치(AttributeSet)의 쿨타임 감소 가져오기
	float CooldownReduction = 0.f;
	if (AttributeSet)
	{
		CooldownReduction = AttributeSet->GetCooldownReduction();
	}

	// 쿨타임 감소 비율은 최대 95%까지만 적용
	CooldownReduction = FMath::Clamp(CooldownReduction, 0.f, 0.95f);

	// 스킬 기본 쿨타임 * 쿨타임 감소 비율로 최종 쿨타임 계산
	const float FinalInterval = BaseCoolTime * (1.f - CooldownReduction);

	// 스킬의 최소 쿨타임은 0.05초로 제한
	return FMath::Max(0.05f, FinalInterval);
}

void AMSPlayerCharacter::TriggerAbilityEvent(const FGameplayTag& EventTag)
{
	// 유효성 검사
	if (!EventTag.IsValid()) return;

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
	if (!EventTag.IsValid()) return;

	// 이벤트 태그 전달
	FGameplayEventData Payload;
	Payload.EventTag = EventTag;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, EventTag, Payload);
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

void AMSPlayerCharacter::GivePlayerStartAbilities_Server()
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
	// 시작 스킬 획득
	for (FStartSkillData StartSkillData : PlayerData.StartSkillDatas)
	{
		if (*StartSkillData.SkillAbilty)
		{
			// PlayerState 가져오기
			AMSPlayerState* PS = GetPlayerState<AMSPlayerState>();
			if (!PS) return;
			
			PS->FindSkillRowBySkillIDAndAdd(StartSkillData.SkillId);
			
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(StartSkillData.SkillAbilty));
			AcquireSkill(StartSkillData.SkillId);
		}
	}
}

void AMSPlayerCharacter::ApplyPlayerStartEffects_Server()
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

void AMSPlayerCharacter::OnExperiencePickupSphereBeginOverlap_Server(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;

	// 경험치 오브 액터로 캐스팅
	AMSExperienceOrb* Orb = Cast<AMSExperienceOrb>(OtherActor);
	if (!Orb) return;

	// 서버에서만 수집 처리(오브 내부에서 중복 획득 방지)
	Orb->Collect_Server(this);
}

void AMSPlayerCharacter::UpdateExperiencePickupRange()
{
	if (!HasAuthority()) return;

	if (!AbilitySystemComponent || !AttributeSet) return;

	// 중복 바인딩 방지
	if (AttributeSetPickupRangeModChangedHandle.IsValid())
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetPickupRangeModAttribute())
			.Remove(AttributeSetPickupRangeModChangedHandle);
		AttributeSetPickupRangeModChangedHandle.Reset();
	}

	// 플레이어 능력치(AttributeSet)의 획득 반경 보정 속성 변경 델리게이트 바인딩
	AttributeSetPickupRangeModChangedHandle = AbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetPickupRangeModAttribute())
		.AddUObject(this, &AMSPlayerCharacter::OnPickupRangeModChanged);
}

void AMSPlayerCharacter::OnPickupRangeModChanged(const FOnAttributeChangeData& Data)
{
	if (!HasAuthority()) return;

	// 플레이어 능력치(AttributeSet)의 획득 반경 보정 가져오기
	float PickupRangeMod = 0.f;
	if (AttributeSet)
	{
		PickupRangeMod = AttributeSet->GetPickupRangeMod();
	}

	// 기본 경험치 픽업 반경 * 획득 반경 보정 비율로 최종 경험치 픽업 반경 계산
	const float FinalExperiencePickupRange
		= BaseExperiencePickupRange * (1.f + PickupRangeMod);

	// 경험치 픽업 반경 업데이트
	ExperiencePickupCollision->SetSphereRadius(FinalExperiencePickupRange);
}

void AMSPlayerCharacter::InitPublicHUDData_Server()
{
	if (!HasAuthority() || !HUDDataComponent) return;

	// 플레이어 이름 및 아이콘 초기화
	if (APlayerState* PS = GetPlayerState())
	{
		HUDDataComponent->BindDisplayName_Server(FText::FromString(PS->GetPlayerName()));
	}
	HUDDataComponent->BindPortraitIcon_Server(PortraitIcon);
}

void AMSPlayerCharacter::BindOverheadNameToHUDData()
{
	if (!HUDDataComponent) return;

	// 중복 바인딩 방지
	if (OverheadDisplayNameChangedHandle.IsValid())
	{
		HUDDataComponent->OnDisplayNameChanged.Remove(OverheadDisplayNameChangedHandle);
		OverheadDisplayNameChangedHandle.Reset();
	}

	// DisplayName 변경 이벤트 델리게이트 바인딩
	OverheadDisplayNameChangedHandle = HUDDataComponent->OnDisplayNameChanged
		.AddUObject(this, &AMSPlayerCharacter::HandleOverheadDisplayNameChanged);

	// 초기값(이미 복제 완료된 경우 포함) 반영
	RefreshOverheadVisibility();
	RefreshOverheadName(&HUDDataComponent->GetDisplayName());
}

void AMSPlayerCharacter::RefreshOverheadVisibility()
{
	if (!OverheadNameWidgetComp) return;

	// 머리 위 이름을 자기 자신은 제외. 로컬에서만 숨김 처리
	const bool bHideForLocal = IsLocallyControlled();
	OverheadNameWidgetComp->SetHiddenInGame(bHideForLocal);
}

void AMSPlayerCharacter::RefreshOverheadName(const FText* OptionalName)
{
	// 숨겨져있으면 종료
	auto bIsHidden = OverheadNameWidgetComp->bHiddenInGame;
	if (!OverheadNameWidgetComp || bIsHidden) return;

	UMSOverheadNameWidget* Widget = Cast<UMSOverheadNameWidget>(OverheadNameWidgetComp->GetUserWidgetObject());
	if (!Widget) return;

	const FText NameToSet =
		OptionalName ? *OptionalName :
		(HUDDataComponent ? HUDDataComponent->GetDisplayName() : FText::GetEmpty());

	Widget->SetNameText(NameToSet);
}

void AMSPlayerCharacter::HandleOverheadDisplayNameChanged(const FText& NewName)
{
	RefreshOverheadVisibility();
	RefreshOverheadName(&NewName);
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

void AMSPlayerCharacter::ClientRPCPlayHealthShake_Implementation(float Scale)
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC || !PC->IsLocalController()) return;

	// 서버로부터 카메라 흔들림을 수행하라는 명령이 떨어지면, 카메라 흔들림 수행
	if (PC->PlayerCameraManager && CameraShakeClass)
	{
		PC->PlayerCameraManager->StartCameraShake(CameraShakeClass, Scale);
	}
}

void AMSPlayerCharacter::SetCharacterOnDead_Server()
{
	// 서버 전용 + 중복 방지
	if (!HasAuthority() || bIsDead) return;

	// 월드, GS, 사망 지점 가져오기
	UWorld* World = GetWorld();
	AMSGameState* GS = World ? World->GetGameState<AMSGameState>() : nullptr;
	const FVector DeathLocation = GetActorLocation();

	// ============================================================
	// #1: 사망 진입(서버): 충돌/이동/스킬/입력 차단 + Dead 태그 부여
	// ============================================================
	OnDeathEnter_Server();


	// ============================================================
	// #2: 현재 세션이 1인인지/멀티인지, 그리고 살아있는 팀원이 있는지 판단
	//    - 멀티(2인 이상): 부활용 액터를 통한 부활 로직을 통해 부활
	//    - 1인(혼자): 부활 로직 수행이 불가능하므로 공유 목숨을 소비하고 즉시 부활
	// ============================================================
	int32 PlayerCountWithPawn = 0;
	bool bHasAliveTeammate = false;
	if (World)
	{
		// 월드 내 PlayerContoller 탐색
		for (APlayerController* PC : TActorRange<APlayerController>(GetWorld()))
		{
			if (!PC) continue;

			// 팀원 카운트 증가
			AMSPlayerCharacter* OtherChar = Cast<AMSPlayerCharacter>(PC->GetPawn());
			if (!OtherChar) continue;
			++PlayerCountWithPawn;

			// 살아있는 팀원 플래그 활성화
			if (OtherChar != this && !OtherChar->GetIsDead())
			{
				bHasAliveTeammate = true;
			}
		}
	}

	// 멀티 플레이인지 플래그로 저장
	const bool bIsMultiplayer = (PlayerCountWithPawn >= 2);


	// ============================================================
	// #3: 멀티 플레이(2인 이상)
	//    - 살아있는 팀원이 없으면: 팀 전멸 -> 게임 패배 처리
	//    - 살아있는 팀원이 있으면: 관전 진입
	//    - 부활 로직을 위한 부활용 액터 스폰
	// ============================================================
	if (bIsMultiplayer)
	{
		// 살아있는 팀원이 없는 경우 -> 팀 전멸 처리
		if (!bHasAliveTeammate)
		{
			if (GS)
			{
				GS->OnSharedLivesDepleted.Broadcast();
			}
			return;
		}

		// 관전 진입	(부활 가능/불가와 무관하게 사망자는 항상 관전)
		bIsSpectating = true;

		// 리슨 서버(호스트)는 OnRep가 안 타므로 즉시 입력/IMC 갱신
		ApplyLocalDeathState(true);

		// 부활용 액터를 스폰
		if (World && !PendingReviveActor && PendingReviveActorClass)
		{
			// 스폰 파라미터 설정
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			// 사망 지점에 마커가 살짝 깔리도록 -Z 오프셋
			FVector SpawnLoc = DeathLocation + FVector(0.f, 0.f, -100.f);

			// 부활용 액터 스폰 후 저장
			PendingReviveActor = World->SpawnActor<AMSTeamReviveActor>(PendingReviveActorClass, SpawnLoc, FRotator::ZeroRotator, Params);
			if (PendingReviveActor)
			{
				// 부활용 액터 초기화 및 부활 진행 시간 설정
				PendingReviveActor->Initialize(this, 2.5f);
			}
		}

		// 관전 대상으로 카메라 전환
		BeginSpectate_Server();
		return;
	}


	// ============================================================
	// 4) 1인 플레이(혼자)
	//    - 공유 목숨이 남아 있으면 즉시 소비하고 사망 지점에서 부활
	//    - 공유 목숨이 0이면 게임 패배 처리
	// ============================================================
	// GameState로부터 공유 목숨 가져오기
	if (GS && GS->GetSharedLives() > 0)
	{
		// 즉시 부활
		ResetCharacterOnRespawn();
		SetActorLocation(DeathLocation);
		return;
	}

	// 남은 목숨 없음 -> 게임 패배 처리
	if (GS)
	{
		GS->OnSharedLivesDepleted.Broadcast();
	}
}

void AMSPlayerCharacter::ResetCharacterOnRespawn()
{
	if (!HasAuthority()) return;

	// 사망한 캐릭터의 월드 및 GameState 가져오기
	UWorld* World = GetWorld();
	AMSGameState* GS = World ? World->GetGameState<AMSGameState>() : nullptr;
	if (!GS) return;

	// #1: 공유 목숨 소비
	GS->ConsumeLife_Server();

	// #2: 서버: 사망에서 꺼둔 요소들 복구
	OnRespawnExit_Server();

	// #3: 카메라를 다시 내 캐릭터로 복귀
	if (AMSPlayerController* MSPC = Cast<AMSPlayerController>(GetController()))
	{
		MSPC->SetSpectateViewTarget(this);
	}
	else if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetViewTargetWithBlend(this, 0.35f, VTBlend_Cubic, 1.5f, false);
	}


	// ============================================================
	// #4: ASC 관련 초기화 로직
	// ============================================================
	// 체력을 최대값으로 회복
	if (AttributeSet)
	{
		AttributeSet->SetHealth(AttributeSet->GetMaxHealth());
	}

	// 모든 어빌리티 쿨타임 초기화
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->CancelAllAbilities(nullptr);
	}
}

void AMSPlayerCharacter::BeginSpectate_Server()
{
	if (!HasAuthority()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;
	AMSPlayerController* MSPC = Cast<AMSPlayerController>(PC);
	if (!MSPC) return;

	// 월드 내 PlayerContoller 탐색
	for (APlayerController* OtherPC : TActorRange<APlayerController>(GetWorld()))
	{
		// 관전 대상이 있는지 확인 후 최초 대상 설정. 해당 대상으로 ViewTarget 설정
		AMSPlayerCharacter* OtherChar = OtherPC ? Cast<AMSPlayerCharacter>(OtherPC->GetPawn()) : nullptr;
		if (OtherChar && OtherChar != this && !OtherChar->bIsDead)
		{
			// 관전 카메라 전환은 PlayerController에서 블렌드 파라미터로 통일
			if (MSPC)
			{
				MSPC->SetSpectateViewTarget(OtherChar);
			}
			else
			{
				PC->SetViewTargetWithBlend(this, 0.35f, VTBlend_Cubic, 1.5f, false);
			}
			break;
		}
	}
}

void AMSPlayerCharacter::OnDeathEnter_Server()
{
	if (!HasAuthority()) return;

	bIsDead = true;

	SetActorHiddenInGame(true);

	// 원본 복구용 현재 상태 캐시 저장
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		CachedCapsuleCollision = Capsule->GetCollisionEnabled();
		CachedCapsuleProfileName = Capsule->GetCollisionProfileName();
		bCachedCapsuleOverlap = Capsule->GetGenerateOverlapEvents();

		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Capsule->SetGenerateOverlapEvents(false);
	}

	// 경험치 픽업 오버랩 비활성화
	if (ExperiencePickupCollision)
	{
		CachedPickupCollision = ExperiencePickupCollision->GetCollisionEnabled();
		bCachedPickupOverlap = ExperiencePickupCollision->GetGenerateOverlapEvents();
		ExperiencePickupCollision->SetGenerateOverlapEvents(false);
		ExperiencePickupCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 이동 정지
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		CachedMovementMode = MoveComp->MovementMode;
		MoveComp->StopMovementImmediately();
		MoveComp->DisableMovement();
	}

	// 스킬/지속기 정리 및 사망 태그 부여
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->CancelAllAbilities(nullptr);
		AbilitySystemComponent->AddLooseGameplayTag(MSGameplayTags::Player_State_Dead);
	}

	// 리슨 서버(호스트) 같은 경우 서버에서 바로 로컬 입력도 차단
	ApplyLocalDeathState(true);

	ForceNetUpdate();
}

void AMSPlayerCharacter::OnRespawnExit_Server()
{
	if (!HasAuthority()) return;

	bIsDead = false;
	bIsSpectating = false;
	PendingReviveActor = nullptr;

	SetActorHiddenInGame(false);

	// 원본 복구
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		if (!CachedCapsuleProfileName.IsNone())
		{
			Capsule->SetCollisionProfileName(CachedCapsuleProfileName);
		}
		Capsule->SetCollisionEnabled(CachedCapsuleCollision);
		Capsule->SetGenerateOverlapEvents(bCachedCapsuleOverlap);
	}

	// 경험치 픽업 오버랩 복구
	if (ExperiencePickupCollision)
	{
		ExperiencePickupCollision->SetCollisionEnabled(CachedPickupCollision);
		ExperiencePickupCollision->SetGenerateOverlapEvents(bCachedPickupOverlap);
	}

	// 이동 복구
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetMovementMode(CachedMovementMode);
	}

	// 사망 상태 태그 제거
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(MSGameplayTags::Player_State_Dead);
	}

	// 로컬 입력 복구
	ApplyLocalDeathState(false);

	ForceNetUpdate();
}

void AMSPlayerCharacter::ApplyLocalDeathState(bool bNowDead)
{
	if (!IsLocallyControlled()) return;

	// IMC 교체
	if (AMSPlayerController* MSPC = Cast<AMSPlayerController>(GetController()))
	{
		MSPC->ApplyLocalInputState(bNowDead);
		return;
	}
}

void AMSPlayerCharacter::OnRep_IsDead()
{
	ApplyLocalDeathState(bIsDead);
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
