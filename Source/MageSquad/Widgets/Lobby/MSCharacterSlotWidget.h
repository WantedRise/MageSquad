// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MSCharacterSlotWidget.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCharacterSlotClicked, FName);
DECLARE_MULTICAST_DELEGATE_OneParam(
    FOnCharacterClicked,
    TSubclassOf<class AMSPlayerCharacter>
);

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
    void InitSlot(FName InCharacterID,const FMSCharacterData& Data);
    void InitSlot(TSubclassOf<AMSPlayerCharacter> InClass);
    void HiddenPortrait();
    FOnCharacterSlotClicked OnClicked;
    FOnCharacterClicked OnCharacterClicked;
protected:
    UPROPERTY(meta = (BindWidget))
    class UImage* Image_Portrait;

    UFUNCTION()
    void HandleClicked();

private:
    FName CharacterID;
    int32 CharacterIndex;
    TSubclassOf<class AMSPlayerCharacter> PawnClass;
};
