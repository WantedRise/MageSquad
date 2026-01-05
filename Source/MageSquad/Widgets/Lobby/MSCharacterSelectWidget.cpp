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

void UMSCharacterSelectWidget::NativeConstruct()
{
    Super::NativeConstruct();

    BuildCharacterSlots();
    //UpdatePlayerState();
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
        CharacterSlot->InitSlot(CharacterData->GetAllCharacter()[Index]);
        CharacterSlot->OnCharacterClicked.AddUObject(this, &UMSCharacterSelectWidget::OnCharacterSlotClicked);
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

void UMSCharacterSelectWidget::OnCharacterSlotClicked(
    TSubclassOf<AMSPlayerCharacter> ClickedPawnClass
)
{
    if (!ClickedPawnClass)
        return;

    // 같은 캐릭터 재클릭 방지
    if (SelectedPawnClass == ClickedPawnClass)
        return;

    // 1️⃣ 로컬 선택 상태 갱신 (UI 즉시 반응)
    SelectedPawnClass = ClickedPawnClass;

    //UMSCharacterDataSubsystem* CharacterData = GetGameInstance()->GetSubsystem<UMSCharacterDataSubsystem>();
    //if (CharacterData)
    //{
    //    const auto* Data = CharacterData->FindCharacterData(CharacterID);
    //    if (Data)
    //    {
    //        Text_Name->SetText(Data->CharacterName);
    //        Text_Desc->SetText(Data->CharacterInfo);
    //        UE_LOG(LogTemp, Log, TEXT("OnCharacterClicked Data"));
    //        UE_LOG(LogTemp, Log, TEXT("OnCharacterClicked ServerSelectCharacter"));
    //    }
    //}

    AMSLobbyPlayerController* PC = GetOwningPlayer<AMSLobbyPlayerController>();
    if (!PC) return;
    PC->Server_SelectCharacter(ClickedPawnClass);
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


void UMSCharacterSelectWidget::HandleCharacterChanged(FName CharacterID)
{
    InfoWidget->Update(CharacterID);
}

