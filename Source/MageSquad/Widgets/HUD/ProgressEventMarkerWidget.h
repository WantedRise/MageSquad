// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ProgressEventMarkerWidget.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UProgressEventMarkerWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget))
	class UImage* Image_Background;
	UPROPERTY(meta = (BindWidget))
	class UImage* Image_Border;
	UPROPERTY(meta = (BindWidget))
	class UImage* Image_Icon;
};
