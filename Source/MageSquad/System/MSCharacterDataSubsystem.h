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

    const FName* FindCharacterID(const FUniqueNetIdRepl& NetId) const;

public:
    FName GetDefaultCharacterID() const;
    const TArray<struct FMSCharacterSelection>& GetAllCharacter() const;

    void CacheSelectedCharacter(const FUniqueNetIdRepl& NetId, FName CharacterID);
    const FMSCharacterSelection* FindSelectionByCharacterId(FName InCharacterId) const;
    const FMSCharacterSelection* FindSelectionByNetId(const FUniqueNetIdRepl& NetId);
public: // 로비에서 NetId ↔ SlotIndex 매핑을 관리
    // ===== 조회 =====
    const int32* FindSlotIndex(const FUniqueNetIdRepl& NetId) const;
    bool IsSlotOccupied(int32 SlotIndex) const;
    // ===== 할당 =====
    bool Assign(const FUniqueNetIdRepl& NetId, int32 SlotIndex);
    // ===== 해제 =====
    void ReleaseByNetId(const FUniqueNetIdRepl& NetId);
    void ReleaseBySlotIndex(int32 SlotIndex);
    // ===== 초기화 =====
    void ClearAll();
private:
    /** DT_Character */
    UPROPERTY()
    UDataTable* CharacterDataTable;

    UPROPERTY(EditDefaultsOnly)
    TArray<FMSCharacterSelection> PlayerCharacterClasses;
    // 플레이어별 선택 상태
    TMap<FUniqueNetIdRepl, FName> SelectedCharacterIDByNetId;
    // 플레이어별 로비에서의 위치
    TMap<FUniqueNetIdRepl, int32> NetIdToSlotIndex;
};