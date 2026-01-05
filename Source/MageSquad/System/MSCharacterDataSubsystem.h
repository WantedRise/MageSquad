// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataStructs/MSCharacterData.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MSCharacterDataSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UMSCharacterDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
    /** Subsystem 초기화 */
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

public:
    /** CharacterID → Character Data */
    const FMSCharacterData* FindCharacterData(FName CharacterID) const;

    FName GetDefaultCharacterID() const;

    /** 전체 캐릭터 데이터 반환 (UI용) */
    const TMap<FName, const FMSCharacterData*>& GetAllCharacters() const
    {
        return CachedCharacterData;
    }

private:
    /** DT_Character */
    UPROPERTY()
    UDataTable* CharacterDataTable;

    /** 캐시 (CharacterID → Row Ptr) */
    TMap<FName, const FMSCharacterData*> CachedCharacterData;

private:
    void LoadCharacterDataTable();
    void BuildCache();
};