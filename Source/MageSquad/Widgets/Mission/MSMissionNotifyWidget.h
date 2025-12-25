// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MSMissionNotifyWidget.generated.h"

/**
 * 작성자: 이상준
 * 작성일: 25/12/23
 *
 * 미션 알림 및 결과를 화면에 표시하는 UI 위젯
 * 미션 진행 중 (미션 시작, 종료)를 텍스트 + 애니메이션으로 표시
 * 
 */
UCLASS()
class MAGESQUAD_API UMSMissionNotifyWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetMissionMessage(FText InMessage);
	void PlayNotify(FText InMessage);
	void PlayMissionResult(bool bSuccess);
public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_MissionMessage;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	class UWidgetAnimation* Anim_ShowNotify;
	
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	class UWidgetAnimation* Anim_MissionResult;
};
