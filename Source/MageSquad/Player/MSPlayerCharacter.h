// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Abilities/GameplayAbility.h"
#include "InputActionValue.h"
#include "Types/MageSquadTypes.h"
#include "MSPlayerCharacter.generated.h"

/**
 * 작성자: 김준형
 * 작성일: 25/12/08
 *
 * 플레이어 캐릭터 클래스
 * - 기본 이동 및 스킬 자동 발사
 * - 이동 스킬(점멸)
 * - 경험치 / 스킬 슬롯 시스템
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

	// 스킬 입력 함수
	void UseBlink(const FInputActionValue& Value);
	void UseLeftSkill(const FInputActionValue& Value);
	void UseRightSkill(const FInputActionValue& Value);


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
	* Attack Section
	*****************************************************/
public:
	// 자동 공격 시작 함수
	UFUNCTION(BlueprintCallable, Category = "Custom | Attack")
	void StartAutoAttack();

	// 자동 공격 멈춤 함수
	UFUNCTION(BlueprintCallable, Category = "Custom | Attack")
	void StopAutoAttack();

private:
	// 자동 공격 호출 요청 함수
	void SetAutoAttackEnabledInternal(bool bEnabled);

	// 자동 공격 호출 함수 (서버 전용)
	void HandleAutoAttack_Server();

	// 서버에게 자동 공격 호출 요청 ServerRPC
	UFUNCTION(Server, Reliable)
	void ServerRPCSetAutoAttackEnabled(bool bEnabled);

protected:
	// 자동 공격 주기
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Attack")
	float AutoAttackInterval = 0.6f;

	// 이 값이 true이면, 서버는 폰이 빙의되는 즉시 자동 공격을 활성화 함
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Attack")
	bool bAutoAttackEnabledOnSpawn = true;

private:
	// 자동 공격 상태
	// 서버 상태. 클라이언트가 필요에 따라 UI를 반영할 수 있도록 복제
	UPROPERTY(Replicated)
	bool bAutoAttackEnabled = false;

	// 자동 공격 타이머
	FTimerHandle AutoAttackTimerHandle;



	/*****************************************************
	* Gameplay Event Trigger Section
	*****************************************************/
private:
	// 어빌리티 트리거 함수
	void TriggerAbilityEvent(const FGameplayTag& EventTag);

	// 서버에게 게임플레이 이벤트를 요청하는 ServerRPC
	UFUNCTION(Server, Reliable)
	void ServerRPCTriggerAbilityEvent(FGameplayTag EventTag);

	// 유효성 검사 함수 (태그 유효 여부)
	bool IsAllowedAbilityEventTag(const FGameplayTag& EventTag) const;



	/*****************************************************
	* Gameplay Event Tag Section
	*****************************************************/
protected:
	// 기본 공격 이벤트 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Tags")
	FGameplayTag AttackStartedEventTag;

	// 점멸 이벤트 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Tags")
	FGameplayTag BlinkEventTag;

	// 패시브 스킬1 이벤트 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Tags")
	FGameplayTag Passive_Skill_01_EventTag;


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
	void GivePlayerStartAbilities();

	// 플레이어 기본 이펙트 적용 함수 (서버 전용)
	void ApplyPlayerStartEffects();

private:
	// 실제 인스턴스는 PlayerState가 소유하며, 캐릭터는 포인터만 참조
	// PossessedBy/OnRep_PlayerState에서 할당된다.

	UPROPERTY(Transient)
	TObjectPtr<class UMSPlayerAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(Transient)
	TObjectPtr<class UMSPlayerAttributeSet> AttributeSet;



	/*****************************************************
	* UI / HUD Section
	*****************************************************/
protected:
	// HUD 공개 데이터 초기화 함수 (서버 전용)
	void InitPublicHUDData_Server();

protected:
	// 플레이어 아이콘 (팀원에게만 표시)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | UI")
	TObjectPtr<UTexture2D> PortraitIcon = nullptr;

	// 서버에서 받는 최소 HUD 공개 데이터 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom | UI")
	TObjectPtr<class UMSHUDDataComponent> HUDDataComponent = nullptr;



	/*****************************************************
	* Damaged Section
	*****************************************************/
public:
	void OnInvincibilityChanged(const FGameplayTag CallbackTag, int32 NewCount);
	void SetInvincibleCollision(bool bInvincible);
};
