// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Lobby/MSCharacterSlotWidget.h"
#include "DataStructs/MSCharacterData.h"
#include "Components/Image.h"
#include <Player/MSLobbyCharacter.h>
#include <System/MSCharacterDataSubsystem.h>
#include "DataAssets/Player/DA_CharacterData.h"
#include <Player/MSPlayerCharacter.h>


void UMSCharacterSlotWidget::InitSlot(FName InCharacterID)
{
    CharacterID = InCharacterID;

    if (UGameInstance* GI = GetGameInstance())
    {
        if (UMSCharacterDataSubsystem* CharacterData = GI->GetSubsystem<UMSCharacterDataSubsystem>())
        {
            const FMSCharacterSelection* Selection = CharacterData->FindSelectionByCharacterId(CharacterID);

            if (!Selection || !Selection->PlayerCharacterClass)
                return;

            //Pawn CDO에서 DataAsset 가져오기
            const AMSPlayerCharacter* PlayerCDO =Cast<AMSPlayerCharacter>(Selection->PlayerCharacterClass->GetDefaultObject());

            if (!PlayerCDO)
                return;

            auto* PlayerStartUpData = PlayerCDO->GetPlayerStartUpData();

            if (!PlayerStartUpData)
                return;

            if (Image_Portrait && PlayerStartUpData->Portrait)
            {
                Image_Portrait->SetBrushFromTexture(
                    PlayerStartUpData->Portrait
                );
            }
        }
    }
}

void UMSCharacterSlotWidget::HiddenPortrait()
{
    Image_Portrait->SetVisibility(ESlateVisibility::Hidden);
}

FReply UMSCharacterSlotWidget::NativeOnMouseButtonDown(
    const FGeometry& InGeometry,
    const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        OnClicked.Broadcast(CharacterID);
        return FReply::Handled();
    }

    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}
