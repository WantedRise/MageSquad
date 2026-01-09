// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/LobbyReadyTypes.h"
#include "MSLobbyReadyWidget.generated.h"

/*
* 작성자: 이상준
* 작성일: 2025-12-16
* 로비 화면에서 플레이어의 준비 상태를 입력 및 표시하기 위한 UI 위젯.
*
* - 준비 버튼 입력을 처리하여 서버에 준비/취소 요청 전달
* - 로비의 현재 준비 단계(미준비 / 일부 준비 / 전체 준비)에 따라 버튼 텍스트 및 카운트다운 UI를 갱신
* - 남은 준비 시간 변경 시(GameState) UI 갱신
*
* 실제 준비 상태 판단 및 카운트다운 제어는 서버(GameMode / GameState)에서 수행
*/
UCLASS()
class MAGESQUAD_API UMSLobbyReadyWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	//위젯 생성 시 버튼 이벤트 및 GameState 델리게이트 바인딩
	virtual void NativeConstruct() override;
public:
	//카운트다운 텍스트 갱신
	UFUNCTION()
	void ApplyReadyTimeText(int32 RemainingSeconds);
	//로비 준비 단계 변경 시 UI 상태를 갱신
	UFUNCTION()
	void ApplyReadyStateUI(ELobbyReadyPhase NewLobbyReadyPhase);
protected:
	//준비 버튼 클릭 시 호출
	UFUNCTION()
	void OnReadyButtonClicked();

public:
	UPROPERTY(BlueprintReadWrite,meta = (BindWidget))
	class UButton* Button_Ready;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_Ready_Default;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_Ready_Selected;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_Ready_Second;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	class USoundWave* ReadyButtonSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	class USoundWave* CancelButtonSound;
private:
	//로컬 플레이어의 준비 상태 (UI 토글용, 서버 상태와 별도)
	bool  bReady;
};
