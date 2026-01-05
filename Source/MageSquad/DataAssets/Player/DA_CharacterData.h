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
USTRUCT()
struct FMSCharacterSelection
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, Category = "CharacterSelection")
    FName CharacterID;

    // 실제 인게임에서 사용할 Pawn
    UPROPERTY(EditDefaultsOnly, Category = "CharacterSelection")
    TSubclassOf<class AMSPlayerCharacter> PlayerCharacterClass;

    // 로비에서 사용할 미리보기 Pawn
    UPROPERTY(EditDefaultsOnly, Category = "CharacterSelection")
    TSubclassOf<class AMSLobbyCharacter> LobbyCharacterClass;
};


UCLASS()
class MAGESQUAD_API UDA_CharacterData : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    // 에디터에서 편집할 캐릭터 클래스 배열
    UPROPERTY(EditDefaultsOnly, Category = "Character")
    TArray<FMSCharacterSelection> CharacterClasses;
};
