// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "InputActionValue.h"
#include "GameplayTagContainer.h"
#include "DataAssets/Player/DA_PlayerStartUpData.h"
#include "Abilities/GameplayAbility.h"
#include "MSPlayerCharacter.generated.h"

class UMSVFXSFXBudgetManager;
class AMSBaseProjectile;
class UMSBlinkAbilityComponent;

/**
 * 작성자: 김준형
 * 작성일: 25/12/08
 *
 * 플레이어 캐릭터 클래스
 * - 기본 이동 및 기본 공격 자동 발사
 * - 이동 스킬(점멸)
 * - 경험치 / 스킬 슬롯 시스템
 * - GAS를 통한 어빌리티 및 능력치(스탯) 관리
 */
UCLASS()
class MAGESQUAD_API AMSPlayerCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AMSPlayerCharacter();

	virtual void PostLoad() override;
	virtual void BeginPlay() override;
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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom | Camera")
	TObjectPtr<class UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom | Camera")
	TObjectPtr<class USpringArmComponent> SpringArm;



	/*****************************************************
	* Input Section
	*****************************************************/
protected:
	virtual void PawnClientRestart() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void Move(const FInputActionValue& Value);

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> MoveAction;



	/*****************************************************
	* Attack Section
	*****************************************************/
public:
	// 자동 공격 함수
	void AutoAttack();

protected:
	// 자동 공격 발사 지연 타이머
	float AutoAttackCooldown;
	float TimeSinceLastAttack;

	// 자동 공격 중인지 여부
	bool bAutoAttacking = false;



	/*****************************************************
	* Player Data Section
	*****************************************************/
public:
	// 플레이어 시작 데이터 Getter (복사 반환)
	UFUNCTION(BlueprintCallable, Category = "Custom | GAS")
	FStartAbilityData GetPlayerStartAbilityData() const { return PlayerStartAbilityData; }

	// 플레이어 시작 데이터 Setter
	UFUNCTION(BlueprintCallable, Category = "Custom | GAS")
	void SetPlayerStartAbilityData(const FStartAbilityData& InPlayerStartData);

protected:
	// 플레이어 시작 데이터 OnRep 함수 (클라이언트 동기화)
	UFUNCTION()
	void OnRep_PlayerStartAbilityData();

	// 플레이어 시작 데이터에서 추가 초기화를 하고 싶을 때 사용할 함수
	virtual void InitFromPlayerStartAbilityData(const FStartAbilityData& InPlayerStartData, bool bFromReplication = false);

protected:
	// 플레이어 시작 데이터 에셋
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | GAS")
	UDA_PlayerStartUpData* PlayerStartUpDataAsset;

	// 네트워크를 통해 동기화되는 플레이어 시작 데이터
	UPROPERTY(ReplicatedUsing = OnRep_PlayerStartAbilityData)
	FStartAbilityData PlayerStartAbilityData;



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
	// ASC
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom | GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UMSPlayerAbilitySystemComponent> AbilitySystemComponent;

	// 플레이어 능력치(스탯)
	UPROPERTY(Transient)
	TObjectPtr< class UMSPlayerAttributeSet> AttributeSet;
};
