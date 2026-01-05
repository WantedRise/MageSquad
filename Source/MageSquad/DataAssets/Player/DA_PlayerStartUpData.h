// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Types/MageSquadTypes.h"
#include "DA_PlayerStartUpData.generated.h"

/**
 * 작성자: 김준형
 * 작성일: 25/12/08
 *
 * 플레이어 시작 데이터 에셋 클래스
 * 플레이어가 게임 시작 시 부여받는 기본 어빌리티 / 이펙트를 정의하는 데이터 에셋
 */
UCLASS()
class MAGESQUAD_API UDA_PlayerStartUpData : public UDataAsset
{
	GENERATED_BODY()

public:
	// 플레이어 시작 어빌리티/이펙트 데이터
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | StartUp")
	FPlayerStartAbilityData PlayerStartAbilityData;
};
