// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "MS_PlayerCharacter.generated.h"

/**
 * 작성자: 김준형
 * 작성일: 25/12/08
 * 
 * 플레이어 캐릭터 클래스
 * 기본 이동, 자동 스킬 사용, 이동기(점멸), 경험치 시스템 등을 담당
 */
UCLASS()
class MAGESQUAD_API AMS_PlayerCharacter : public ACharacter/*, public IAbilitySystemInterface*/
{
	GENERATED_BODY()

public:
	AMS_PlayerCharacter();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;


};
