// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Lobby/MSCharacterSlotWidget.h"
#include "DataStructs/MSCharacterData.h"
#include "Components/Image.h"

void UMSCharacterSlotWidget::InitSlot(FName InCharacterID, const FMSCharacterData& Data)
{
    CharacterID = InCharacterID;
    if (Data.Portrait)
    {
        Image_Portrait->SetBrushFromTexture(Data.Portrait);
    }
}

void UMSCharacterSlotWidget::HiddenPortrait()
{
    Image_Portrait->SetVisibility(ESlateVisibility::Hidden);
}

void UMSCharacterSlotWidget::HandleClicked()
{
    OnClicked.Broadcast(CharacterID);
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
