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
	void Update(FName CharacterID);
};
