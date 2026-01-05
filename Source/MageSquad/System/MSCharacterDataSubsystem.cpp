// Fill out your copyright notice in the Description page of Project Settings.


#include "System/MSCharacterDataSubsystem.h"

void UMSCharacterDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

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