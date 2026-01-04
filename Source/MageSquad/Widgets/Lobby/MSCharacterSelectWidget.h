// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MSCharacterSelectWidget.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UMSCharacterSelectWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

private:
    void BuildCharacterSlots();
    void OnCharacterClicked(FName CharacterID);
    void HandleCharacterChanged(FName CharacterID);
    //void BindPlayerState();
protected:
    UPROPERTY(meta = (BindWidget))
    class UUniformGridPanel* CharacterGrid;
    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<class UMSCharacterSlotWidget> SlotWidgetClass;
    UPROPERTY(meta = (BindWidget))
    class UMSCharacterInfoWidget* InfoWidget;
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* Text_Name;
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* Text_Desc;

    UPROPERTY(EditDefaultsOnly, Category = "Grid")
    int32 Columns = 4;

    UPROPERTY(EditDefaultsOnly, Category = "Grid")
    int32 Rows = 1;
        
};
