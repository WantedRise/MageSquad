// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MSCharacterSlotWidget.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCharacterSlotClicked, FName);

struct FMSCharacterData;

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UMSCharacterSlotWidget : public UUserWidget
{
	GENERATED_BODY()
public:
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    void SetSelectedSlot(bool bSelected);
    FName GetCharacterID() { return CharacterID; }
public:
    void InitSlot(FName InCharacterID);
    void HiddenPortrait();
    FOnCharacterSlotClicked OnClicked;

protected:
    UPROPERTY(meta = (BindWidget))
    class UImage* Image_Portrait;
    UPROPERTY(meta = (BindWidget))
    class UBorder* Border_Select;

private:
    FName CharacterID;
};
