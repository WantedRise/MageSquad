// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "InputActionValue.h"
#include "GameplayTagContainer.h"
#include "MSPlayerCharacter.generated.h"

/**
 * 작성자: 김준형
 * 작성일: 25/12/08
 *
 * 플레이어 캐릭터 클래스
 * 기본 이동, 자동 스킬 사용, 이동기(점멸), 경험치 시스템 등을 담당
 */
UCLASS()
class MAGESQUAD_API AMSPlayerCharacter : public ACharacter/*, public IAbilitySystemInterface*/
{
	GENERATED_BODY()

//public:
//	AMS_PlayerCharacter();
//
//	virtual void PostInitializeComponents() override;
//	virtual void PostLoad() override;
//
//	virtual void Tick(float DeltaTime) override;
//
//
//protected:
//	virtual void BeginPlay() override;
//	virtual void PossessedBy(AController* NewController) override;
//
//	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
//
//
//	/*****************************************************
//	* Character Section
//	*****************************************************/
//private:
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
//	TObjectPtr<class UCameraComponent> Camera;
//
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
//	TObjectPtr<class USpringArmComponent> SpringArm;
//
//
//	/*****************************************************
//	* Input Section
//	*****************************************************/
//protected:
//	virtual void PawnClientRestart() override;
//	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
//
//private:
//	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Input", meta = (AllowPrivateAccess = "true"))
//	TObjectPtr<class UInputMappingContext> DefaultMappingContext;
//
//	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Input", meta = (AllowPrivateAccess = "true"))
//	TObjectPtr<class UInputAction> MoveAction;
//
//
//	/*****************************************************
//	* Ability System Section
//	*****************************************************/
//public:
//	// Pawn의 상태 변경 리플리케이션 함수
//	virtual void OnRep_PlayerState() override;
//
//	// GAS Getter
//	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
//
//	// AttributeSet Getter
//	virtual class UMS_PlayerAttributeSet* GetAttributeSet() const;
//
//private:
//	// ASC
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom | ASC", meta = (AllowPrivateAccess = true))
//	TObjectPtr<class UMS_PlayerAbilitySystemComponent> AbilitySystemComponent;
//
//	// AttributeSet. ASC에서 사용하는 능력치 집합
//	UPROPERTY(Transient)
//	TObjectPtr<class UMS_PlayerAttributeSet> AttributeSet;
//
//	// 플레이어 캐릭터 시작 게임플레이 약 참조 데이터 (GA/GE)
//	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | StartUpData", meta = (AllowPrivateAccess = true))
//	TSoftObjectPtr<class UDA_PlayerStartUpData> CharacterStartUpData;
};
