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
public:
    void InitSlot(FName InCharacterID);
    void HiddenPortrait();
    FOnCharacterSlotClicked OnClicked;

protected:
    UPROPERTY(meta = (BindWidget))
    class UImage* Image_Portrait;

private:
    FName CharacterID;
    int32 CharacterIndex;
    TSubclassOf<class AMSLobbyCharacter> PawnClass;
};
