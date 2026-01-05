// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MSLobbyMainWidget.generated.h"

class UMSCharacterSelectWidget;

/*
* 작성자: 이상준
* 작성일: 2025-12-16
* 로비에서 메인 위젯을 담당
* 
* - 준비 버튼 UI
* 
*/
UCLASS()
class MAGESQUAD_API UMSLobbyMainWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UMSLobbyMainWidget(const FObjectInitializer& ObjectInitializer);
	UMSCharacterSelectWidget* GetCharacterSelectWidget() { return MSCharacterSelect; }
public:
	UPROPERTY(meta = (BindWidget))
	class UMSLobbyReadyWidget* WBP_MSLobbyReady;
	UPROPERTY(meta = (BindWidget))
	UMSCharacterSelectWidget* MSCharacterSelect;
};
