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
    const TArray<TSubclassOf<class AMSPlayerCharacter>>& GetAllCharacter() const;
    const TMap<FUniqueNetIdRepl, TSubclassOf<class AMSPlayerCharacter>>& GetAllPlayerCharacter() const;

    /** 로비에서 캐릭터 선택 저장 */
    void CacheSelectedCharacterForPlayer(const FUniqueNetIdRepl& NetId, TSubclassOf<class AMSPlayerCharacter> SelectedClass);

    /** 인게임에서 캐릭터 선택 복원 */
    bool PopCachedSelectedCharacterForPlayer(const FUniqueNetIdRepl& NetId, TSubclassOf<class AMSPlayerCharacter>& OutSelectedClass);
    void CacheSelectedCharacterForLobby(const FUniqueNetIdRepl& NetId, TSubclassOf<class AMSLobbyCharacter> SelectedClass);
    bool PopCachedSelectedCharacterForLobby(const FUniqueNetIdRepl& NetId, TSubclassOf<class AMSLobbyCharacter>& OutSelectedClass);
private:
    /** DT_Character */
    UPROPERTY()
    UDataTable* CharacterDataTable;

    /** 캐시 (CharacterID → Row Ptr) */
    TMap<FName, const FMSCharacterData*> CachedCharacterData;
    TArray<TSubclassOf<class AMSPlayerCharacter>> PlayerCharacterClasses;
    TArray<TSubclassOf<class AMSLobbyCharacter>> LobbyCharacterClasses;
    TMap<FUniqueNetIdRepl, TSubclassOf<class AMSPlayerCharacter>> CachedSelectedCharacters;
    TMap<FUniqueNetIdRepl, TSubclassOf<class AMSLobbyCharacter>> CachedSelectedLobbyCharacters;
private:
    void LoadCharacterDataTable();
    void BuildCache();
};