// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MSGameInstance.generated.h"

/**
 * 게임 레벨에서 사용하는 게임 인스턴스 클래스
 */
UCLASS()
class MAGESQUAD_API UMSGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	// 게임 인스턴스 초기화 함수
	virtual void Init() override;
};
