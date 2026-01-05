// Fill out your copyright notice in the Description page of Project Settings.


#include "System/MSCharacterDataSubsystem.h"
#include "DataAssets/Player/DA_CharacterData.h"
#include "DataAssets/Player/DA_LobbyCharacterData.h"
#include "Player/MSPlayerCharacter.h"
#include "Player/MSLobbyCharacter.h"
void UMSCharacterDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    FString CharacterAssetPath = TEXT("/Game/Data/Player/DA_CharacterData.DA_CharacterData");
    UDA_CharacterData* CharacterDataAsset = Cast<UDA_CharacterData>(StaticLoadObject(UDA_CharacterData::StaticClass(), nullptr, *CharacterAssetPath));
    if (CharacterDataAsset)
    {
        PlayerCharacterClasses = CharacterDataAsset->CharacterClasses;
    }
    FString LobbyCharacterAssetPath = TEXT("/Game/Data/Player/DA_LobbyCharacterData.DA_LobbyCharacterData");
    UDA_LobbyCharacterData* LobbyCharacterDataAsset = Cast<UDA_LobbyCharacterData>(StaticLoadObject(UDA_LobbyCharacterData::StaticClass(), nullptr, *LobbyCharacterAssetPath));
    if (LobbyCharacterDataAsset)
    {
        LobbyCharacterClasses = LobbyCharacterDataAsset->CharacterClasses;
    }
    LoadCharacterDataTable();
    BuildCache();
}

void UMSCharacterDataSubsystem::Deinitialize()
{
    CachedCharacterData.Empty();
    CharacterDataTable = nullptr;

    Super::Deinitialize();
}

void UMSCharacterDataSubsystem::LoadCharacterDataTable()
{
    static const FString Context(TEXT("CharacterDataSubsystem"));
    static const TCHAR* DataTablePath = TEXT("/Game/Data/Character/DT_MSCharacter.DT_MSCharacter");

    CharacterDataTable = LoadObject<UDataTable>(nullptr, DataTablePath);

    if (!CharacterDataTable)
    {
        UE_LOG(LogTemp, Error,
            TEXT("[CharacterDataSubsystem] Failed to load DT_Character"));
    }

    
}

void UMSCharacterDataSubsystem::BuildCache()
{
    if (!CharacterDataTable)
        return;

    CachedCharacterData.Empty();

    TArray<FName> RowNames = CharacterDataTable->GetRowNames();
    for (const FName& RowName : RowNames)
    {
        const FMSCharacterData* Row =
            CharacterDataTable->FindRow<FMSCharacterData>(
                RowName,
                TEXT("BuildCharacterCache")
            );

        if (!Row)
            continue;

        CachedCharacterData.Add(RowName, Row);
    }

    UE_LOG(LogTemp, Log, TEXT("[CharacterDataSubsystem] Loaded %d characters "), CachedCharacterData.Num());
}

 const FMSCharacterData* UMSCharacterDataSubsystem::FindCharacterData(FName CharacterID) const
{
     if (const FMSCharacterData *const * Found = CachedCharacterData.Find(CharacterID))
     {
         return *Found;
     }

    return nullptr;
}

 FName UMSCharacterDataSubsystem::GetDefaultCharacterID() const
 {
     for (const auto& Pair : CachedCharacterData)
     {
         return Pair.Key;
     }

     return NAME_None;
 }

 const TArray<TSubclassOf<class AMSPlayerCharacter>>& UMSCharacterDataSubsystem::GetAllCharacter() const
 {
     return PlayerCharacterClasses;
 }

 const TMap<FUniqueNetIdRepl, TSubclassOf<class AMSPlayerCharacter>>& UMSCharacterDataSubsystem::GetAllPlayerCharacter() const
 {
     return CachedSelectedCharacters;
 }

 void UMSCharacterDataSubsystem::CacheSelectedCharacterForPlayer(const FUniqueNetIdRepl& NetId, TSubclassOf<class AMSPlayerCharacter> SelectedClass)
 {
     if (!NetId.IsValid() || !SelectedClass)
     {
         UE_LOG(LogTemp, Warning,
             TEXT("CacheSelectedCharacterForPlayer failed (Invalid NetId or Class)"));
         return;
     }

     CachedSelectedCharacters.FindOrAdd(NetId) = SelectedClass;
 }

 bool UMSCharacterDataSubsystem::PopCachedSelectedCharacterForPlayer(const FUniqueNetIdRepl& NetId, TSubclassOf<class AMSPlayerCharacter>& OutSelectedClass)
 {
     if (!NetId.IsValid())
         return false;

     if (TSubclassOf<AMSPlayerCharacter>* Found =
         CachedSelectedCharacters.Find(NetId))
     {
         OutSelectedClass = *Found;
         CachedSelectedCharacters.Remove(NetId);

         return true;
     }

     return false;
 }
 void UMSCharacterDataSubsystem::CacheSelectedCharacterForLobby(const FUniqueNetIdRepl& NetId, TSubclassOf<class AMSLobbyCharacter> SelectedClass)
 {
     if (!NetId.IsValid() || !SelectedClass)
     {
         UE_LOG(LogTemp, Warning,
             TEXT("CacheSelectedCharacterForPlayer failed (Invalid NetId or Class)"));
         return;
     }

     CachedSelectedLobbyCharacters.FindOrAdd(NetId) = SelectedClass;
 }

 bool UMSCharacterDataSubsystem::PopCachedSelectedCharacterForLobby(const FUniqueNetIdRepl& NetId, TSubclassOf<class AMSLobbyCharacter>& OutSelectedClass)
 {
     if (!NetId.IsValid())
         return false;

     if (TSubclassOf<AMSLobbyCharacter>* Found =
         CachedSelectedLobbyCharacters.Find(NetId))
     {
         OutSelectedClass = *Found;
         CachedSelectedLobbyCharacters.Remove(NetId);

         return true;
     }

     return false;
 }