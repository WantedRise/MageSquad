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
    FName GetDefaultCharacterID() const;
    const TArray<struct FMSCharacterSelection>& GetAllCharacter() const;

    void CacheSelectedCharacter(const FUniqueNetIdRepl& NetId, FName CharacterID);
    const FMSCharacterSelection* FindSelectionByCharacterId(FName InCharacterId) const;
    const FMSCharacterSelection* FindSelectionByNetId(const FUniqueNetIdRepl& NetId);
private:
    /** DT_Character */
    UPROPERTY()
    UDataTable* CharacterDataTable;

    UPROPERTY(EditDefaultsOnly)
    TArray<FMSCharacterSelection> PlayerCharacterClasses;
    // 플레이어별 선택 상태
    TMap<FUniqueNetIdRepl, FName> SelectedCharacterIDByNetId;
};