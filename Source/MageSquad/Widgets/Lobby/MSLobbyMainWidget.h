// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MSLobbyMainWidget.generated.h"

class UMSCharacterSelectWidget;

enum class EButtonState
{
	Lobby,
	SelectCharacter
};

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
	virtual void NativeConstruct() override;
protected:
	UFUNCTION()
	void OnClickedSelectCharacterButton();
	UFUNCTION()
	void OnHoveredSelectCharacterButton();
	UFUNCTION()
	void OnUnHoveredSelectCharacterButton();
	UFUNCTION()
	void OnClickedLobbyButton();
	UFUNCTION()
	void OnHoveredLobbyButton();
	UFUNCTION()
	void OnUnHoveredLobbyButton();
	UFUNCTION()
	void OnExitClicked();
	void SetScaleButton(class UButton* InButton, float SetSize);
public:
	UPROPERTY(meta = (BindWidget))
	class UMSLobbyReadyWidget* WBP_MSLobbyReady;
	UPROPERTY(meta = (BindWidget))
	UMSCharacterSelectWidget* MSCharacterSelect;
	UPROPERTY(meta = (BindWidget))
	class UButton* Button_Select;
	UPROPERTY(meta = (BindWidget))
	class UButton* Button_Lobby;
	UPROPERTY(meta = (BindWidget))
	class UButton* Button_Exit;
	
	EButtonState ButtonState = EButtonState::Lobby;
};
