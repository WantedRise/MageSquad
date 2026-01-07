// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MSGameInstance.generated.h"

/**
 * ���� �������� ����ϴ� ���� �ν��Ͻ� Ŭ����
 */
UCLASS()
class MAGESQUAD_API UMSGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
};
