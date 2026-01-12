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

const FName* UMSCharacterDataSubsystem::FindCharacterID(const FUniqueNetIdRepl& NetId) const
{
    if (!NetId.IsValid())
        return nullptr;

    return SelectedCharacterIDByNetId.Find(NetId);
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

 void UMSCharacterDataSubsystem::CacheSelectedCharacter(const FUniqueNetIdRepl& NetId,FName CharacterID)
 {
     if (!NetId.IsValid() || CharacterID.IsNone())
         return;

     SelectedCharacterIDByNetId.FindOrAdd(NetId) = CharacterID;
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
     if (!TempCharacterID) return nullptr;
     for (const FMSCharacterSelection& Selection : PlayerCharacterClasses)
     {
         if (Selection.CharacterID == *TempCharacterID)
         {
             return &Selection;
         }
     }
     return nullptr;
 }

 const int32* UMSCharacterDataSubsystem::FindSlotIndex(const FUniqueNetIdRepl& NetId) const
 {
     if (!NetId.IsValid())
         return nullptr;

     return NetIdToSlotIndex.Find(NetId);
 }

 bool UMSCharacterDataSubsystem::IsSlotOccupied(int32 SlotIndex) const
 {
     return NetIdToSlotIndex.FindKey(SlotIndex) != nullptr;
 }

 bool UMSCharacterDataSubsystem::Assign(const FUniqueNetIdRepl& NetId, int32 SlotIndex)
 {
     if (!NetId.IsValid())
         return false;

     // 이미 이 NetId가 슬롯을 가지고 있으면 실패
     if (NetIdToSlotIndex.Contains(NetId))
         return false;

     // 이미 다른 플레이어가 쓰는 슬롯이면 실패
     if (IsSlotOccupied(SlotIndex))
         return false;

     NetIdToSlotIndex.Add(NetId, SlotIndex);
     return true;
 }

 void UMSCharacterDataSubsystem::ReleaseByNetId(const FUniqueNetIdRepl& NetId)
 {
     if (!NetId.IsValid())
         return;

     NetIdToSlotIndex.Remove(NetId);
 }

 void UMSCharacterDataSubsystem::ReleaseBySlotIndex(int32 SlotIndex)
 {
     if (const FUniqueNetIdRepl* FoundKey = NetIdToSlotIndex.FindKey(SlotIndex))
     {
         NetIdToSlotIndex.Remove(*FoundKey);
     }
 }

 void UMSCharacterDataSubsystem::ClearAll()
 {
     NetIdToSlotIndex.Empty();
 }