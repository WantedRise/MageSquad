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
 * �ۼ���: ������
 * �ۼ���: 25/12/08
 *
 * �÷��̾� ĳ���� Ŭ����
 * - �⺻ �̵� �� ��ų �ڵ� �߻�
 * - �̵� ��ų(����)
 * - ����ġ / ��ų ���� �ý���
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
	* PlayerState OnRep �Լ�
	* PlayerState�� Ŭ���̾�Ʈ�� �����Ǿ��� ��, ȣ��
	* Ŭ���̾�Ʈ �� ASC �ʱ�ȭ�� ���
	*/
	virtual void OnRep_PlayerState() override;



	/*****************************************************
	* Player Section
	*****************************************************/
protected:
	// ������ �޽�
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom | Weapon")
	TObjectPtr<class UStaticMeshComponent> StaffMesh;

	// ������ �޽ø� ������ ���� �̸�
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Weapon")
	FName StaffAttachSocketName = TEXT("s_Staff");



	/*****************************************************
	* Camera Section
	*****************************************************/
protected:
	// ī�޶� �� ������Ʈ �Լ�
	void UpdateCameraZoom(float DeltaTime);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom | Camera")
	TObjectPtr<class UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom | Camera")
	TObjectPtr<class USpringArmComponent> SpringArm;

	// ���� ī�޶� ���� ��ǥ ���� (SpringArm TargetArmLength�� ��ǥ��)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom | Camera")
	float TargetArmLength = 2000.f;

	// ī�޶� �� �ּ� �Ÿ�
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Camera")
	float MinCameraZoomLength = 1000.f;

	// ī�޶� �� �ִ� �Ÿ�
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Camera")
	float MaxCameraZoomLength = 2500.f;

	// ���콺 �� �� ���� ����Ǵ� �Ÿ� ��
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Camera")
	float CameraZoomStep = 300.f;

	// �� ��/�ƿ� ���� �ӵ� (���� Ŭ���� ������ ����)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Camera")
	float CameraZoomInterpSpeed = 4.f;



	/*****************************************************
	* Input Section
	*****************************************************/
protected:
	virtual void PawnClientRestart() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// �̵�, ī�޶� �� ��/�ƿ� �Լ�
	void Move(const FInputActionValue& Value);
	void CameraZoom(const FInputActionValue& Value);

	// ��ų �Է� �Լ�
	void UseBlink(const FInputActionValue& Value);
	void UseLeftSkill(const FInputActionValue& Value);
	void UseRightSkill(const FInputActionValue& Value);


	// TEST: HP ����/���� �Լ�
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


	// TEST: HP ����/���� �Է� �׼�
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
	// �ڵ� ���� ���� �Լ�
	UFUNCTION(BlueprintCallable, Category = "Custom | Attack")
	void StartAutoAttack();

	// �ڵ� ���� ���� �Լ�
	UFUNCTION(BlueprintCallable, Category = "Custom | Attack")
	void StopAutoAttack();

private:
	// �ڵ� ���� ȣ�� ��û �Լ�
	void SetAutoAttackEnabledInternal(bool bEnabled);

	// �ڵ� ���� ȣ�� �Լ� (���� ����)
	void HandleAutoAttack_Server();

	// �������� �ڵ� ���� ȣ�� ��û ServerRPC
	UFUNCTION(Server, Reliable)
	void ServerRPCSetAutoAttackEnabled(bool bEnabled);

protected:
	// �ڵ� ���� �ֱ�
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Attack")
	float AutoAttackInterval = 0.6f;

	// �� ���� true�̸�, ������ ���� ���ǵǴ� ��� �ڵ� ������ Ȱ��ȭ ��
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Attack")
	bool bAutoAttackEnabledOnSpawn = true;

private:
	// �ڵ� ���� ����
	// ���� ����. Ŭ���̾�Ʈ�� �ʿ信 ���� UI�� �ݿ��� �� �ֵ��� ����
	UPROPERTY(Replicated)
	bool bAutoAttackEnabled = false;

	// �ڵ� ���� Ÿ�̸�
	FTimerHandle AutoAttackTimerHandle;



	/*****************************************************
	* Gameplay Event Trigger Section
	*****************************************************/
private:
	// �����Ƽ Ʈ���� �Լ�
	void TriggerAbilityEvent(const FGameplayTag& EventTag);

	// �������� �����÷��� �̺�Ʈ�� ��û�ϴ� ServerRPC
	UFUNCTION(Server, Reliable)
	void ServerRPCTriggerAbilityEvent(FGameplayTag EventTag);

	// ��ȿ�� �˻� �Լ� (�±� ��ȿ ����)
	bool IsAllowedAbilityEventTag(const FGameplayTag& EventTag) const;



	/*****************************************************
	* Gameplay Event Tag Section
	*****************************************************/
protected:
	// �⺻ ���� �̺�Ʈ �±�
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Tags")
	FGameplayTag AttackStartedEventTag;

	// ���� �̺�Ʈ �±�
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Tags")
	FGameplayTag BlinkEventTag;

	// �нú� ��ų1 �̺�Ʈ �±�
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Tags")
	FGameplayTag Passive_Skill_01_EventTag;


	// TEST: HP ����/���� �̺�Ʈ �±�
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
	// �÷��̾� ������ Getter
	UFUNCTION(BlueprintCallable, Category = "Custom | Player")
	FPlayerStartAbilityData GetPlayerData() const { return PlayerData; }

	// �÷��̾� ������ Setter
	UFUNCTION(BlueprintCallable, Category = "Custom | Player")
	void SetPlayerData(const FPlayerStartAbilityData& InPlayerData);

protected:
	// �÷��̾� ���� ������ ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Player")
	TObjectPtr<class UDA_PlayerStartUpData> PlayerStartUpData;

	// �÷��̾� ������
	UPROPERTY(Replicated)
	FPlayerStartAbilityData PlayerData;



	/*****************************************************
	* Ability System Section
	*****************************************************/
public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// �ڽſ��� �����÷��� ����Ʈ�� �����ϴ� ���� �Լ�
	bool ApplyGameplayEffectToSelf(TSubclassOf<class UGameplayEffect> Effect, FGameplayEffectContextHandle InEffectContextHandle);

	// �÷��̾� �⺻ �����Ƽ �ο� �Լ� (���� ����)
	void GivePlayerStartAbilities();

	// �÷��̾� �⺻ ����Ʈ ���� �Լ� (���� ����)
	void ApplyPlayerStartEffects();

private:
	// ���� �ν��Ͻ��� PlayerState�� �����ϸ�, ĳ���ʹ� �����͸� ����
	// PossessedBy/OnRep_PlayerState���� �Ҵ�ȴ�.

	UPROPERTY(Transient)
	TObjectPtr<class UMSPlayerAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(Transient)
	TObjectPtr<class UMSPlayerAttributeSet> AttributeSet;

	/*****************************************************
	* UI / HUD Section
	*****************************************************/
protected:
	// HUD ���� ������ �ʱ�ȭ �Լ� (���� ����)
	void InitPublicHUDData_Server();

protected:
	// �÷��̾� ������ (�������Ը� ǥ��)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | UI")
	TObjectPtr<UTexture2D> PortraitIcon = nullptr;

	// �������� �޴� �ּ� HUD ���� ������ ������Ʈ
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom | UI")
	TObjectPtr<class UMSHUDDataComponent> HUDDataComponent = nullptr;
	
	/*****************************************************
	* Damaged Section
	*****************************************************/
public:
	void OnInvincibilityChanged(const FGameplayTag CallbackTag, int32 NewCount);
	void SetInvincibleCollision(bool bInvincible);
	
};
