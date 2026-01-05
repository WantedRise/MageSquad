// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DA_CharacterData.generated.h"

/**
 * 작성자: 이상준
 * 작성일: 25/01/05
 *
 * 플레이어 캐릭터 종류
 */
UCLASS()
class MAGESQUAD_API UDA_CharacterData : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    // 에디터에서 편집할 캐릭터 클래스 배열
    UPROPERTY(EditDefaultsOnly, Category = "Character")
    TArray<TSubclassOf<class AMSPlayerCharacter>> CharacterClasses;
};
