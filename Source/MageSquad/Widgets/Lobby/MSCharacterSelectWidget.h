// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MSCharacterSelectWidget.generated.h"

struct FMSCharacterData;
struct FMSSkillList;
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
    void GetSkillData();
    void BuildCharacterSlots();
    void OnCharacterSlotClicked(FName InCharacterId);
    const FMSSkillList* FindSkillRows(int32 InSkillID);
protected:
    UPROPERTY(meta = (BindWidget))
    class UUniformGridPanel* CharacterGrid;
    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<class UMSCharacterSlotWidget> SlotWidgetClass;
    UPROPERTY(meta = (BindWidget))
    class UMSCharacterInfoWidget* SelectCharacterInfoWidget;
    UPROPERTY(meta = (BindWidget))
    class UMSCharacterInfoWidget* PassiveSkillInfoWidget;
    UPROPERTY(meta = (BindWidget))
    class UMSCharacterInfoWidget* ActiveSkillLeftInfoWidget;
    UPROPERTY(meta = (BindWidget))
    class UMSCharacterInfoWidget* ActiveSkillRightInfoWidget;

    UPROPERTY(EditDefaultsOnly, Category = "Grid")
    int32 Columns = 4;

    UPROPERTY(EditDefaultsOnly, Category = "Grid")
    int32 Rows = 1;
    
    FName CurrentCharacterID;
    // 스킬 데이터 테이블
    UPROPERTY(EditDefaultsOnly, Category = "Skill")
    class UDataTable* SkillListDataTable;

    TArray<FMSSkillList*> AllSkillRows;
};
