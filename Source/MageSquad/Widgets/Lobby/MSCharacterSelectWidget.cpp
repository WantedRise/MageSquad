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

void UMSCharacterSelectWidget::NativeConstruct()
{
    Super::NativeConstruct();

    BuildCharacterSlots();
    UpdatePlayerState();
}

void UMSCharacterSelectWidget::BuildCharacterSlots()
{
    UMSCharacterDataSubsystem* CharacterData = GetGameInstance()->GetSubsystem<UMSCharacterDataSubsystem>();

    const auto& Characters = CharacterData->GetAllCharacters();

    const int32 TotalSlots = Columns * Rows;
    int32 Index = 0;
    for (const auto& Pair : Characters)
    {
        FName CharacterID = Pair.Key;
        const FMSCharacterData* Data = Pair.Value;

        UMSCharacterSlotWidget* CharacterSlot = CreateWidget<UMSCharacterSlotWidget>(this, SlotWidgetClass);
        CharacterGrid->AddChildToUniformGrid(CharacterSlot, Index / Columns, Index % Columns);
      
            
        CharacterSlot->InitSlot(CharacterID, *Data);
        CharacterSlot->OnClicked.AddUObject(this, &UMSCharacterSelectWidget::OnCharacterClicked);
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

void UMSCharacterSelectWidget::OnCharacterClicked(FName CharacterID)
{
    UE_LOG(LogTemp, Log, TEXT("OnCharacterClicked"));
    AMSLobbyPlayerController* PC = GetOwningPlayer<AMSLobbyPlayerController>();
    if (!PC) return;

    UMSCharacterDataSubsystem* CharacterData = GetGameInstance()->GetSubsystem<UMSCharacterDataSubsystem>();
    if (CharacterData)
    {
        const auto* Data = CharacterData->FindCharacterData(CharacterID);
        if(Data)
        {
            Text_Name->SetText(Data->CharacterName);
            Text_Desc->SetText(Data->CharacterInfo);
            UE_LOG(LogTemp, Log, TEXT("OnCharacterClicked Data"));

            if (CurrentCharacterID != CharacterID)
            {
                PC->ApplyCharacterPreview(*Data);
            }
            PC->ServerSelectCharacter(CharacterID);
            UE_LOG(LogTemp, Log, TEXT("OnCharacterClicked ServerSelectCharacter"));
        }
    }

    CurrentCharacterID = CharacterID;
}

void UMSCharacterSelectWidget::ApplySelectedCharacter(FMSCharacterData Data)
{
    /*
    // 1️⃣ 슬롯 UI 선택 상태 갱신
    for (UMSCharacterSlotWidget* Slot : AllSlots)
    {
        if (!Slot)
            continue;

        const bool bIsSelected =
            (Slot->GetCharacterID() == CharacterID);

        Slot->SetSelected(bIsSelected);
    }

    // 2️⃣ 우측 정보 패널 갱신
    if (InfoWidget)
    {
        InfoWidget->Update(CharacterID);
    }
    */
}

void UMSCharacterSelectWidget::UpdatePlayerState()
{
    AMSLobbyPlayerState* PS = GetOwningPlayerState<AMSLobbyPlayerState>();

    if (!PS) return;

    // 2️⃣ ⭐ 이미 값이 있으면 즉시 UI에 반영
    const FName SelectedID = PS->GetSelectedCharacterID();
    if (SelectedID != NAME_None)
    {
        UE_LOG(LogTemp, Log, TEXT("UpdatePlayerState"));
        OnCharacterClicked(SelectedID);
    }
}


void UMSCharacterSelectWidget::HandleCharacterChanged(FName CharacterID)
{
    InfoWidget->Update(CharacterID);
}

