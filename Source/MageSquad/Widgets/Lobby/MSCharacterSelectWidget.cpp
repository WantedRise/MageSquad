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

void UMSCharacterSelectWidget::NativeConstruct()
{
    Super::NativeConstruct();

    BuildCharacterSlots();
    //UpdatePlayerState();
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
    if (NAME_None == InCharacterId)
        return;

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
    UE_LOG(LogTemp, Log, TEXT("GetOwningPlayer"));
    AMSLobbyPlayerController* PC = GetOwningPlayer<AMSLobbyPlayerController>();
    if (!PC) return;
    PC->Server_SelectCharacter(InCharacterId);
    UE_LOG(LogTemp, Log, TEXT("GetOwningPlayer aa"));
}

void UMSCharacterSelectWidget::HandleCharacterChanged(FName CharacterID)
{
    InfoWidget->Update(CharacterID);
}

