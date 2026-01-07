// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Abilities/GameplayAbility.h"
#include "InputActionValue.h"
#include "Types/MageSquadTypes.h"
#include "DataAssets/Player/DA_PlayerStartUpData.h"
#include "MSPlayerCharacter.generated.h"

// 스킬 슬롯 업데이트 이벤트 델리게이트
DECLARE_MULTICAST_DELEGATE(FOnSkillSlotsUpdated);

// 스킬 쿨다운 시작 이벤트 델리게이트
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSkillCooldownStarted, uint8 /*SlotIndex*/, float /*Duration*/);

// 블링크 스킬 쿨다운 시작 이벤트 델리게이트
DECLARE_MULTICAST_DELEGATE(FOnBlinkSkillCooldownStarted);

/**
 * 스킬 슬롯 인덱스(고정)
 */
UENUM(BlueprintType)
enum class EMSSkillSlotIndex : uint8
{
	ActiveLeft = 0,		// 좌클릭 액티브 스킬 슬롯
	ActiveRight = 1,	// 우클릭 액티브 스킬 슬롯
	Passive01 = 2,		// 패시브 스킬 1번
	Passive02 = 3,		// 패시브 스킬 2번
	Passive03 = 4,		// 패시브 스킬 3번
	Passive04 = 5,		// 패시브 스킬 4번
};

/**
 * 작성자: 김준형
 * 작성일: 25/12/08
 *
 * 플레이어 캐릭터 클래스
 * - 기본 이동 및 카메라
 * - 공유 경험치/레벨
 * - 전투/비전투 스킬 슬롯 시스템
 */
UCLASS()
class MAGESQUAD_API AMSPlayerCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AMSPlayerCharacter();

	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSecond) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/*
	* PlayerState OnRep 함수
	* PlayerState가 클라이언트에 복제되었을 때, 호출
	* 클라이언트 측 ASC 초기화에 사용
	*/
	virtual void OnRep_PlayerState() override;



	/*****************************************************
	* Player Section
	*****************************************************/
protected:
	// 지팡이 메시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom | Weapon")
	TObjectPtr<class UStaticMeshComponent> StaffMesh;

	// 지팡이 메시를 부착할 소켓 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Weapon")
	FName StaffAttachSocketName = TEXT("s_Staff");



	/*****************************************************
	* Camera Section
	*****************************************************/
protected:
	// 카메라 줌 업데이트 함수
	void UpdateCameraZoom(float DeltaTime);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom | Camera")
	TObjectPtr<class UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom | Camera")
	TObjectPtr<class USpringArmComponent> SpringArm;

	// 현재 카메라 줌의 목표 길이 (SpringArm TargetArmLength의 목표값)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom | Camera")
	float TargetArmLength = 2000.f;

	// 카메라 줌 최소 거리
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Camera")
	float MinCameraZoomLength = 1000.f;

	// 카메라 줌 최대 거리
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Camera")
	float MaxCameraZoomLength = 2500.f;

	// 마우스 휠 한 번당 변경되는 거리 값
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Camera")
	float CameraZoomStep = 300.f;

	// 줌 인/아웃 보간 속도 (값이 클수록 빠르게 따라감)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Camera")
	float CameraZoomInterpSpeed = 4.f;



	/*****************************************************
	* Input Section
	*****************************************************/
protected:
	virtual void PawnClientRestart() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// 이동, 카메라 줌 인/아웃 함수
	void Move(const FInputActionValue& Value);
	void CameraZoom(const FInputActionValue& Value);

	// 이동 스킬 함수
	void UseBlink(const FInputActionValue& Value);

	// 액티브 스킬 입력 함수
	void UseLeftSkill(const FInputActionValue& Value);
	void UseRightSkill(const FInputActionValue& Value);

	// 액티브 스킬(좌/우) 사용 요청 함수 (로컬 입력 -> 서버 RPC)
	void RequestUseActiveSkill(EMSSkillSlotIndex SlotIndex);


	// TEST: HP 증가/감소 함수
	void TEST_HpIncrease(const FInputActionValue& Value);
	void TEST_HpDecrease(const FInputActionValue& Value);
	void TEST_MaxHpIncrease(const FInputActionValue& Value);
	void TEST_MaxHpDecrease(const FInputActionValue& Value);

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> CameraZoomAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> BlinkAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> LeftSkillAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> RightSkillAction;


	// TEST: HP 증가/감소 입력 액션
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> TEST_HpIncreaseAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> TEST_HpDecreaseAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> TEST_MaxHpIncreaseAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> TEST_MaxHpDecreaseAction;



	/*****************************************************
	* Attack Skill Section (Passive/Active)
	*****************************************************/
public:
	// 스킬 획득 함수 (외부 호출 가능)
	UFUNCTION(BlueprintCallable, Category = "Custom | Skill")
	void AcquireSkill(int32 SkillID);

	// 현재 슬롯 정보(OwnerOnly 복제, UI/입력 바인딩 용도)
	UFUNCTION(BlueprintCallable, Category = "Custom | Skill")
	const TArray<FMSPlayerSkillSlotNet>& GetSkillSlots() const { return SkillSlots; }

	// 스킬 런타임 데이터 배열 반환 함수
	const TArray<TObjectPtr<UMSSkillSlotRuntimeData>>& GetSkillRuntimeData() const { return SkillRuntimeData; }

	// 클라이언트에게 로컬 스킬 쿨다운이 시작되었음을 알리는 함수
	UFUNCTION(Client, Reliable)
	void ClientRPCStartSkillCooldown(uint8 SlotIndex, float Duration);

protected:
	// 스킬 슬롯 업데이트 OnRep 함수
	UFUNCTION()
	void OnRep_SkillSlots();

private:
	// 서버: 스킬 획득 내부 처리 함수
	void AcquireSkill_Server(int32 SkillID);

	// 서버에게 스킬 획득을 요청하는 함수 ServerRPC
	UFUNCTION(Server, Reliable)
	void ServerRPCAcquireSkill(int32 SkillID);

	// 스킬 슬롯 보정 함수 (배열/런타임 데이터/타이머)
	void EnsureSkillSlotArrays();


	// 서버: 스킬 슬롯에서 유효한 스킬 이벤트를 발사하는 함수 (발사 결과 반환)
	bool SendSkillActive_Server(int32 SlotIndex);

	// 서버에게 스킬 발사 요청하는 함수 ServerRPC
	UFUNCTION(Server, Reliable)
	void ServerRPCSendSkillActive(uint8 SlotIndex);


	// 서버: SkillType에 맞는 스킬 장착 함수
	void EquipSkillFromRow_Server(const FMSSkillList& Row);

	// 서버: 장착 or 갱신할 패시브 스킬 슬롯 반환 함수
	int32 FindOrAllocatePassiveSlotIndex_Server(int32 SkillID) const;


	// 서버: 패시브 스킬 자동 발동 타이머 재구성 함수
	void RebuildPassiveSkillTimers_Server();

	// 서버: 패시브 스킬 실제 자동 발동 트리거 함수
	void SendPassiveSkillActive_Server(int32 SlotIndex);

	// 패시브 스킬 자동 사용 주기 계산 함수
	float ComputeFinalInterval(float BaseCoolTime) const;


	// 플레이어 능력치(AttributeSet)의 쿨타임 감소 속성 변경 델리게이트 바인딩 함수
	void BindCooldownReductionDelegate();

	// 속성 변경에 따른 콜백 함수
	void OnCooldownReductionChanged(const FOnAttributeChangeData& Data);

public:
	// 스킬 슬롯 업데이트 이벤트 객체
	FOnSkillSlotsUpdated OnSkillSlotsUpdated;

	// 스킬 쿨다운 시작 이벤트 객체
	FOnSkillCooldownStarted OnSkillCooldownStarted;

protected:
	// 스킬 슬롯
	UPROPERTY(ReplicatedUsing = OnRep_SkillSlots)
	TArray<FMSPlayerSkillSlotNet> SkillSlots;

	// 패시브 스킬 타이머 핸들(4개)
	TArray<FTimerHandle> PassiveSkillTimerHandles;

	// 플레이어 능력치(AttributeSet)의 쿨타임 감소 속성 변경 델리게이트 핸들
	FDelegateHandle AttributeSetCooldownReductionChangedHandle;

	// 스킬 런타임 데이터 (서버 전용)
	UPROPERTY(Transient)
	TArray<TObjectPtr<UMSSkillSlotRuntimeData>> SkillRuntimeData;



	/*****************************************************
	* Non Attack Skill Section
	*****************************************************/
private:
	// 어빌리티 트리거 함수
	void TriggerAbilityEvent(const FGameplayTag& EventTag);

	// 서버에게 게임플레이 이벤트를 요청하는 ServerRPC
	UFUNCTION(Server, Reliable)
	void ServerRPCTriggerAbilityEvent(FGameplayTag EventTag);

	// 클라이언트에게 로컬 블링크 스킬 쿨다운이 시작되었음을 알리는 함수
	UFUNCTION(Client, Reliable)
	void ClientRPCStartBlinkSkillCooldown();

public:
	// 블링크 스킬 쿨다운 시작 이벤트 객체
	FOnBlinkSkillCooldownStarted OnBlinkSkillCooldownStarted;

protected:
	// 블링크 스킬 이벤트 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Tags")
	FGameplayTag BlinkEventTag;


	// TEST: HP 증가/감소 이벤트 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Tags")
	FGameplayTag TEST_HpIncreaseEventTag;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Tags")
	FGameplayTag TEST_HpDecreaseEventTag;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Tags")
	FGameplayTag TEST_MaxHpIncreaseEventTag;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Tags")
	FGameplayTag TEST_MaxHpDecreaseEventTag;



	/*****************************************************
	* Player Data Section
	*****************************************************/
public:
	// 플레이어 데이터 Getter
	UFUNCTION(BlueprintCallable, Category = "Custom | Player")
	FPlayerStartAbilityData GetPlayerData() const { return PlayerData; }

	// 플레이어 데이터 Setter
	UFUNCTION(BlueprintCallable, Category = "Custom | Player")
	void SetPlayerData(const FPlayerStartAbilityData& InPlayerData);

	UDA_PlayerStartUpData* GetPlayerStartUpData() const { return PlayerStartUpData; }
protected:
	// 플레이어 시작 데이터 에셋
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Player")
	TObjectPtr<class UDA_PlayerStartUpData> PlayerStartUpData;

	// 플레이어 데이터
	UPROPERTY(Replicated)
	FPlayerStartAbilityData PlayerData;



	/*****************************************************
	* Ability System Section
	*****************************************************/
public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// 자신에게 게임플레이 이펙트를 적용하는 헬퍼 함수
	bool ApplyGameplayEffectToSelf(TSubclassOf<class UGameplayEffect> Effect, FGameplayEffectContextHandle InEffectContextHandle);

	// 플레이어 기본 어빌리티 부여 함수 (서버 전용)
	void GivePlayerStartAbilities_Server();

	// 플레이어 기본 이펙트 적용 함수 (서버 전용)
	void ApplyPlayerStartEffects_Server();

private:
	// 실제 인스턴스는 PlayerState가 소유하며, 캐릭터는 포인터만 참조
	// PossessedBy/OnRep_PlayerState에서 할당된다.

	UPROPERTY(Transient)
	TObjectPtr<class UMSPlayerAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(Transient)
	TObjectPtr<class UMSPlayerAttributeSet> AttributeSet;



	/*****************************************************
	* ItemOrb Section
	*****************************************************/
protected:
	// 서버: 아이템 오브 픽업 오버랩 함수
	UFUNCTION()
	void OnItemOrbPickup_Server(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 플레이어 능력치(AttributeSet)의 획득 반경 보정 속성 변경 델리게이트 바인딩 함수
	void UpdateOrbPickupRange();

	// 속성 변경에 따른 콜백 함수
	void OnPickupRangeModChanged(const FOnAttributeChangeData& Data);

protected:
	// 기본 아이템 오브 픽업 반경
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Experience")
	float BaseItemOrbPickupRange = 500.f;

	// 플레이어 능력치(AttributeSet)의 획득 반경 보정 속성 변경 델리게이트 핸들
	FDelegateHandle AttributeSetPickupRangeModChangedHandle;

	// 아이템 오브 픽업 콜리전
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom | Experience")
	TObjectPtr<class USphereComponent> ItemOrbPickupCollision;



	/*****************************************************
	* UI / HUD / Indicator Section
	*****************************************************/
protected:
	// HUD 공개 데이터 초기화 함수 (서버 전용)
	void InitPublicHUDData_Server();

	// DisplayName 변경 이벤트 바인딩 함수
	void BindOverheadNameToHUDData();

	// 머리 위 이름 텍스트 재가시화 함수
	void RefreshOverheadVisibility();

	// 머리 위 이름 텍스트 재초기화 함수
	void RefreshOverheadName(const FText* OptionalName = nullptr);

	// DisplayName 변경 이벤트 콜백 함수
	void HandleOverheadDisplayNameChanged(const FText& NewName);

protected:
	// 서버에서 받는 최소 HUD 공개 데이터 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom | UI")
	TObjectPtr<class UMSHUDDataComponent> HUDDataComponent = nullptr;

	// 방향 표시 인디케이터 컴포넌트 (팀원에게 위치 및 초상화 제공)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom | Indicator")
	TObjectPtr<class UMSDirectionIndicatorComponent> DirectionIndicatorComponent;

	// 플레이어 아이콘 (팀원에게만 표시)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | UI")
	TObjectPtr<UTexture2D> PortraitIcon = nullptr;

	// 캐릭터 머리 위에 띄울 플레이어 이름 (팀원에게만 표시)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom | UI")
	TObjectPtr<class UWidgetComponent> OverheadNameWidgetComp = nullptr;

	// 머리 위 이름 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | UI")
	TSubclassOf<class UMSOverheadNameWidget> OverheadNameWidgetClass;

	// HUDDataComponent DisplayName 변경 델리게이트 핸들(중복 바인딩/해제 목적)
	FDelegateHandle OverheadDisplayNameChangedHandle;



	/*****************************************************
	* Damaged Section
	*****************************************************/
public:
	void OnInvincibilityChanged(const FGameplayTag CallbackTag, int32 NewCount);
	void SetInvincibleCollision(bool bInvincible);

	void HandleSetInvincibleCollision_Server(bool bInvincible);

	UFUNCTION(Server, Reliable)
	void ServerRPCHandleSetInvincibleCollision(bool bInvincible);

	// 로컬 클라이언트에 카메라 흔들림 수행용 ClientRPC (흔들림은 Unreliable로 충분)
	UFUNCTION(Client, Unreliable)
	void ClientRPCPlayHealthShake(float Scale);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Damaged")
	TSubclassOf<class UCameraShakeBase> CameraShakeClass;



	/*****************************************************
	* Death & Respawn Section
	*****************************************************/
public:
	// 서버: 사망 처리 함수. 현재 체력이 0 이하로 떨어졌을 때 호출됨
	UFUNCTION(BlueprintCallable, Category = "Custom | Respawn")
	virtual void SetCharacterOnDead_Server();

	// 캐릭터 부활 처리 함수. 서버에서 부활 처리가 완료되었을 때 호출됨
	UFUNCTION(BlueprintCallable, Category = "Custom | Respawn")
	virtual void ResetCharacterOnRespawn();

	// 사망, 관전 상태 Getter
	FORCEINLINE const bool GetIsDead() const { return bIsDead; }
	FORCEINLINE const bool GetSpectating() const { return bIsSpectating; }

protected:
	// 서버: 관전 모드 진입 함수
	void BeginSpectate_Server();

	// 서버: 사망 진입 시, 충돌/이동/입력/스킬 등을 비활성화하는 함수
	void OnDeathEnter_Server();

	// 서버: 부활 완료 시, 사망 진입에서 끈 요소들을 복구하는 함수
	void OnRespawnExit_Server();

	// 사망 상태가 변경되었을 때 로컬 클라이언트에서 입력/카메라/UI 등을 처리하는 함수
	void ApplyLocalDeathState(bool bNowDead);

	// 사망 상태 변경 OnRep 함수
	UFUNCTION()
	void OnRep_IsDead();

protected:
	// 사망 상태 여부
	UPROPERTY(ReplicatedUsing = OnRep_IsDead)
	bool bIsDead = false;

	// 관전 상태 여부
	UPROPERTY(Replicated)
	bool bIsSpectating = false;

	// 팀 부활용 액터. 서버만 소유
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Respawn")
	TSubclassOf<class AMSTeamReviveActor> PendingReviveActorClass;

private:
	UPROPERTY(Transient)
	TObjectPtr<class AMSTeamReviveActor> PendingReviveActor;

	/* ======================== Cached Data ======================== */
	UPROPERTY(Transient)
	TEnumAsByte<ECollisionEnabled::Type> CachedCapsuleCollision = ECollisionEnabled::QueryAndPhysics;

	UPROPERTY(Transient)
	FName CachedCapsuleProfileName;

	UPROPERTY(Transient)
	bool bCachedCapsuleOverlap = true;

	UPROPERTY(Transient)
	TEnumAsByte<ECollisionEnabled::Type> CachedPickupCollision = ECollisionEnabled::NoCollision;

	UPROPERTY(Transient)
	bool bCachedPickupOverlap = false;

	UPROPERTY(Transient)
	TEnumAsByte<EMovementMode> CachedMovementMode = MOVE_Walking;
	/* ======================== Cached Data ======================== */
};
