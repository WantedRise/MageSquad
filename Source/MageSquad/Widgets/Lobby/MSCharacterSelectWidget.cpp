// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Lobby/MSCharacterSelectWidget.h"
#include <System/MSCharacterDataSubsystem.h>
#include "MSCharacterSlotWidget.h"
#include "Components/UniformGridPanel.h"
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

    BuildCharacterSlots();
    GetSkillData();
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

void UMSCharacterSelectWidget::BuildCharacterSlots()
{
    UMSCharacterDataSubsystem* CharacterData = GetGameInstance()->GetSubsystem<UMSCharacterDataSubsystem>();
    const auto& CharacterSelections = CharacterData->GetAllCharacter();

    const int32 TotalSlots = Columns * Rows;
    int32 Index = 0;
    for (const FMSCharacterSelection& Selection : CharacterSelections)
    {
        UMSCharacterSlotWidget* CharacterSlot = CreateWidget<UMSCharacterSlotWidget>(this, SlotWidgetClass);
        CharacterGrid->AddChildToUniformGrid(CharacterSlot, Index / Columns, Index % Columns);

        
        CharacterSlot->InitSlot(Selection.CharacterID);
        CharacterSlot->OnClicked.AddUObject(this, &UMSCharacterSelectWidget::OnCharacterSlotClicked);
        Index++;
    }

    for (; Index < TotalSlots; ++Index)
    {
        UMSCharacterSlotWidget* EmptySlot = CreateWidget<UMSCharacterSlotWidget>(this, SlotWidgetClass);

        if (!EmptySlot) continue;
        EmptySlot->HiddenPortrait();
        CharacterGrid->AddChildToUniformGrid(EmptySlot,Index / Columns,Index % Columns);
    }
}

// MSCharacterSelectWidget.cpp

void UMSCharacterSelectWidget::OnCharacterSlotClicked(FName InCharacterId)
{
    if (NAME_None == InCharacterId || CurrentCharacterID == InCharacterId)
        return;
    if (UGameInstance* GI = GetGameInstance())
    {
        if (UMSCharacterDataSubsystem* CharacterData = GI->GetSubsystem<UMSCharacterDataSubsystem>())
        {
            const FMSCharacterSelection* Selection = CharacterData->FindSelectionByCharacterId(InCharacterId);

            if (!Selection || !Selection->PlayerCharacterClass)
                return;

            //Pawn CDO에서 DataAsset 가져오기
            const AMSPlayerCharacter* PlayerCDO = Cast<AMSPlayerCharacter>(Selection->PlayerCharacterClass->GetDefaultObject());

            if (!PlayerCDO)
                return;

            auto* PlayerStartUpData = PlayerCDO->GetPlayerStartUpData();

            if (!PlayerStartUpData)
                return;

            SelectCharacterInfoWidget->UpdateInfoWidget(FText::GetEmpty(), PlayerStartUpData->CharacterName, PlayerStartUpData->InitialStatInfo.Num()>0 ? PlayerStartUpData->InitialStatInfo[0] : FText::GetEmpty(), PlayerStartUpData->Portrait);

            if (PlayerStartUpData->PlayerStartAbilityData.StartSkillDatas.IsValidIndex(0))
            {
                int32 PassiveSkillID = PlayerStartUpData->PlayerStartAbilityData.StartSkillDatas[0].SkillId;
                if (const FMSSkillList* SkillData = FindSkillRows(PassiveSkillID))
                {
                    PassiveSkillInfoWidget->UpdateInfoWidget(FText::FromString(TEXT("패시브 스킬")), FText::FromString(SkillData->SkillName), SkillData->SkillDescription, SkillData->SkillIcon);
                }
            }
            if (PlayerStartUpData->PlayerStartAbilityData.StartSkillDatas.IsValidIndex(1))
            {
                int32 PassiveSkillID = PlayerStartUpData->PlayerStartAbilityData.StartSkillDatas[1].SkillId;
                if (const FMSSkillList* SkillData = FindSkillRows(PassiveSkillID))
                {
                    ActiveSkillLeftInfoWidget->UpdateInfoWidget(FText::FromString(TEXT("액티브 스킬 1")), FText::FromString(SkillData->SkillName), SkillData->SkillDescription, SkillData->SkillIcon);
                }
            }
            if (PlayerStartUpData->PlayerStartAbilityData.StartSkillDatas.IsValidIndex(2))
            {
                int32 PassiveSkillID = PlayerStartUpData->PlayerStartAbilityData.StartSkillDatas[2].SkillId;
                if (const FMSSkillList* SkillData = FindSkillRows(PassiveSkillID))
                {
                    ActiveSkillRightInfoWidget->UpdateInfoWidget(FText::FromString(TEXT("액티브 스킬 2")), FText::FromString(SkillData->SkillName), SkillData->SkillDescription, SkillData->SkillIcon);
                }
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("GetOwningPlayer"));
    AMSLobbyPlayerController* PC = GetOwningPlayer<AMSLobbyPlayerController>();
    if (!PC) return;
    PC->Server_SelectCharacter(InCharacterId);
}

