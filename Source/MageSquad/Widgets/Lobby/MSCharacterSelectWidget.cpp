// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Lobby/MSCharacterSelectWidget.h"
#include <System/MSCharacterDataSubsystem.h>
#include "MSCharacterSlotWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/TextBlock.h"
#include <Player/MSLobbyPlayerController.h>
#include "MSCharacterInfoWidget.h"
#include <Player/MSLobbyPlayerState.h>
#include <Interfaces/CharacterAppearanceInterface.h>
#include "Player/MSPlayerCharacter.h"
#include "DataAssets/Player/DA_CharacterData.h"
#include "SkillData/MSSkillList.h"
#include "Types/MageSquadTypes.h"

void UMSCharacterSelectWidget::NativeConstruct()
{
    Super::NativeConstruct();

    GetSkillData();
    BuildCharacterSlots();
}

void UMSCharacterSelectWidget::GetSkillData()
{
    if (!SkillListDataTable)
        return;

    static const FString Ctx(TEXT("FindSkillRowByTag"));
    SkillListDataTable->GetAllRows(Ctx, AllSkillRows);
}

const FMSSkillList* UMSCharacterSelectWidget::FindSkillRows(int32 InSkillID)
{
    if (AllSkillRows.Num()<=0)
        return nullptr;

    for (const FMSSkillList* Row : AllSkillRows)
    {
        if (Row && Row->SkillID == InSkillID)
        {
            return Row;
        }
    }
    return nullptr;
}

void UMSCharacterSelectWidget::UpdateCharacterInfoByCharacterId(FName CharacterId)
{
    if (CharacterId == NAME_None)
        return;

    UGameInstance* GI = GetGameInstance();
    if (!GI)
        return;

    UMSCharacterDataSubsystem* CharacterData = GI->GetSubsystem<UMSCharacterDataSubsystem>();
    if (!CharacterData)
        return;

    const FMSCharacterSelection* Selection = CharacterData->FindSelectionByCharacterId(CharacterId);

    if (!Selection || !Selection->PlayerCharacterClass)
        return;

    // Pawn CDO에서 DataAsset 가져오기
    const AMSPlayerCharacter* PlayerCDO =
        Cast<AMSPlayerCharacter>(Selection->PlayerCharacterClass->GetDefaultObject());

    if (!PlayerCDO)
        return;

    const UDA_PlayerStartUpData* PlayerStartUpData = PlayerCDO->GetPlayerStartUpData();

    if (!PlayerStartUpData)
        return;

    SetVisibleCharacterSlotBorder(CharacterId);

    /* ===============================
     * 기본 캐릭터 정보
     * =============================== */
    SelectCharacterInfoWidget->UpdateInfoWidget(
        FText::GetEmpty(),
        PlayerStartUpData->CharacterName,
        PlayerStartUpData->InitialStatInfo.IsValidIndex(0)
        ? PlayerStartUpData->InitialStatInfo[0]
        : FText::GetEmpty(),
        PlayerStartUpData->Portrait
    );

    /* ===============================
     * 패시브 스킬
     * =============================== */
    if (PlayerStartUpData->PlayerStartAbilityData.StartSkillDatas.IsValidIndex(0))
    {
        const int32 SkillID =
            PlayerStartUpData->PlayerStartAbilityData.StartSkillDatas[0].SkillId;

        if (const FMSSkillList* SkillData = FindSkillRows(SkillID))
        {
            PassiveSkillInfoWidget->UpdateInfoWidget(
                FText::FromString(TEXT("패시브 스킬")),
                FText::FromString(SkillData->SkillName),
                SkillData->SkillDescription,
                SkillData->SkillIcon
            );
        }
    }

    /* ===============================
     * 액티브 스킬 1
     * =============================== */
    if (PlayerStartUpData->PlayerStartAbilityData.StartSkillDatas.IsValidIndex(1))
    {
        const int32 SkillID =
            PlayerStartUpData->PlayerStartAbilityData.StartSkillDatas[1].SkillId;

        if (const FMSSkillList* SkillData = FindSkillRows(SkillID))
        {
            ActiveSkillLeftInfoWidget->UpdateInfoWidget(
                FText::FromString(TEXT("액티브 스킬 1")),
                FText::FromString(SkillData->SkillName),
                SkillData->SkillDescription,
                SkillData->SkillIcon
            );
        }
    }

    /* ===============================
     * 액티브 스킬 2
     * =============================== */
    if (PlayerStartUpData->PlayerStartAbilityData.StartSkillDatas.IsValidIndex(2))
    {
        const int32 SkillID =
            PlayerStartUpData->PlayerStartAbilityData.StartSkillDatas[2].SkillId;

        if (const FMSSkillList* SkillData = FindSkillRows(SkillID))
        {
            ActiveSkillRightInfoWidget->UpdateInfoWidget(
                FText::FromString(TEXT("액티브 스킬 2")),
                FText::FromString(SkillData->SkillName),
                SkillData->SkillDescription,
                SkillData->SkillIcon
            );
        }
    }
}

void UMSCharacterSelectWidget::BuildCharacterSlots()
{
    
    UMSCharacterDataSubsystem* CharacterData = GetGameInstance()->GetSubsystem<UMSCharacterDataSubsystem>();
    check(CharacterData);
    const auto& CharacterSelections = CharacterData->GetAllCharacter();

    const int32 TotalSlots = Columns * Rows;
    int32 Index = 0;
    for (const FMSCharacterSelection& Selection : CharacterSelections)
    {
        UMSCharacterSlotWidget* CharacterSlot = CreateWidget<UMSCharacterSlotWidget>(this, SlotWidgetClass);
        UUniformGridSlot* NewSlot = CharacterGrid->AddChildToUniformGrid(CharacterSlot, Index / Columns, Index % Columns);

        CharacterSlot->InitSlot(Selection.CharacterID);
        CharacterSlot->OnClicked.AddUObject(this, &UMSCharacterSelectWidget::OnCharacterSlotClicked);
        CharacterSlots.Add(CharacterSlot);
        Index++;
    }

   /* for (; Index < TotalSlots; ++Index)
    {
        UMSCharacterSlotWidget* EmptySlot = CreateWidget<UMSCharacterSlotWidget>(this, SlotWidgetClass);

        if (!EmptySlot) continue;
        EmptySlot->HiddenPortrait();
        CharacterGrid->AddChildToUniformGrid(EmptySlot,Index / Columns,Index % Columns);
    }*/
}


void UMSCharacterSelectWidget::UpdateUI(const FUniqueNetIdRepl NetID)
{
    /*AMSLobbyPlayerController* PC = GetOwningPlayer<AMSLobbyPlayerController>();

    if (!PC) return;

    PC->Server_RequestCharacterID(NetID);
    */

    UMSCharacterDataSubsystem* CharacterData = GetGameInstance()->GetSubsystem<UMSCharacterDataSubsystem>();
    check(CharacterData);

    const FMSCharacterSelection* CharacterSelection = CharacterData->FindSelectionByNetId(NetID);
    if (CharacterSelection)
    {
        UE_LOG(LogTemp, Error, TEXT("FMSCharacterSelection %s"), *CharacterSelection->CharacterID.ToString());
        UpdateCharacterInfoByCharacterId(CharacterSelection->CharacterID);
    }
    else
    {
        const FName DefualtCharacterID = CharacterData->GetDefaultCharacterID();
        if (DefualtCharacterID != NAME_None)
        {
            UE_LOG(LogTemp, Error, TEXT("FMSCharacterSelection %s"), *DefualtCharacterID.ToString());
            UpdateCharacterInfoByCharacterId(DefualtCharacterID);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[UMSCharacterSelectWidget] No CharacterData"));
        }
    }
    UE_LOG(LogTemp, Error, TEXT("[FMSCharacterSelection] ENd"));
}

void UMSCharacterSelectWidget::SetVisibleCharacterSlotBorder(FName InCharacterId)
{
    for (UMSCharacterSlotWidget* CharacterSlot : CharacterSlots)
    {
        const bool bIsSelected = (CharacterSlot->GetCharacterID() == InCharacterId);
        CharacterSlot->SetSelectedSlot(bIsSelected);
    }

    // 선택된 캐릭터 ID 처리
    CurrentCharacterID = InCharacterId;
}

void UMSCharacterSelectWidget::OnCharacterSlotClicked(FName InCharacterId)
{
    if (NAME_None == InCharacterId || CurrentCharacterID == InCharacterId)
        return;

    UpdateCharacterInfoByCharacterId(InCharacterId);

    UE_LOG(LogTemp, Log, TEXT("GetOwningPlayer"));
    AMSLobbyPlayerController* PC = GetOwningPlayer<AMSLobbyPlayerController>();
    if (!PC) return;
    PC->Server_SelectCharacter(InCharacterId);
}

