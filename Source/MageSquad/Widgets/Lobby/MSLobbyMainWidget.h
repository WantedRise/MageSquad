// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
//#include "Widgets/Lobby/MSLobbyReadyWidget.h"
#include "MSLobbyMainWidget.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UMSLobbyMainWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget))
	class UMSLobbyReadyWidget* WBP_MSLobbyReady;
};
