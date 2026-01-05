// Fill out your copyright notice in the Description page of Project Settings.


#include "System/MSCharacterDataSubsystem.h"
#include "DataAssets/Player/DA_CharacterData.h"
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
}

void UMSCharacterDataSubsystem::Deinitialize()
{
    SelectedCharacterIDByNetId.Empty();
    CharacterDataTable = nullptr;

    Super::Deinitialize();
}

 FName UMSCharacterDataSubsystem::GetDefaultCharacterID() const
 {
     for (const auto& CharacterClassData : PlayerCharacterClasses)
     {
         return CharacterClassData.CharacterID;
     }

     return NAME_None;
 }

 const TArray<FMSCharacterSelection>& UMSCharacterDataSubsystem::GetAllCharacter() const
 {
     return PlayerCharacterClasses;
 }

 void UMSCharacterDataSubsystem::CacheSelectedCharacter(const FUniqueNetIdRepl& NetId,FName CharacterId)
 {
     if (!NetId.IsValid() || CharacterId.IsNone())
         return;

     SelectedCharacterIDByNetId.FindOrAdd(NetId) = CharacterId;
 }

 const FMSCharacterSelection* UMSCharacterDataSubsystem::FindSelectionByCharacterId(FName InCharacterId) const
 {
     for (const FMSCharacterSelection& Selection : PlayerCharacterClasses)
     {
         if (Selection.CharacterID == InCharacterId)
         {
             return &Selection;
         }
     }
     return nullptr;
 }

 const FMSCharacterSelection* UMSCharacterDataSubsystem::FindSelectionByNetId(const FUniqueNetIdRepl& NetID)
 {
     const FName* TempCharacterID = SelectedCharacterIDByNetId.Find(NetID);

     for (const FMSCharacterSelection& Selection : PlayerCharacterClasses)
     {
         if (Selection.CharacterID == *TempCharacterID)
         {
             return &Selection;
         }
     }
     return nullptr;
 }
