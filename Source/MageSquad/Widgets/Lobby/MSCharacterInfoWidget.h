// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MSCharacterInfoWidget.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UMSCharacterInfoWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void UpdateInfoWidget(FText Title, FText Name, FText Decs, UTexture2D* Icon);
    void UpdateInfoWidget(FText Title, FText Name, FText Decs, TSoftObjectPtr <class UMaterialInterface> Icon);

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* Text_Decs;
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* Text_Name;
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* Text_Title;
    UPROPERTY(meta = (BindWidget))
    class UImage* Image_Icon;
    UPROPERTY(meta = (BindWidget))
    class UHorizontalBox* HorizontalBox_Title;
    UPROPERTY(meta = (BindWidget))
    class UImage* Image_divider;
    
    UPROPERTY(Transient)
    TObjectPtr<class UMaterialInstanceDynamic> SkillIconMID;
};
