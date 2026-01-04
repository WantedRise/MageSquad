// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MSMissionTrackerWidget.generated.h"

struct FMSMissionProgressUIData;

/**
 * 작성자: 이상준	
 * 작성일: 25/12/30
 *
 * 현재 진행 중인 미션의 상태를 HUD에 표시하는 위젯
 *
 * - 미션 제목 / 설명 텍스트 출력
 * - 제한 시간 타이머 표시
 * - 일반 목표 / 보스 목표에 따른 Progress UI 분기 처리
 */
UCLASS()
class MAGESQUAD_API UMSMissionTrackerWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetMissionTitle(FText InTitle);
	void SetMissionMessage(FText Desc);
	// 미션 제한 시간 타이머 시작
	void StartMissionTimer(class AMSGameState* InGameState,float InEndTime);
	// 서버 시간을 기준으로 남은 시간을 계산하여 UI 갱신
	void UpdateRemainingTime();
	// Todo : HP 분기에 따라 개선
	// 진행도 갱신
	void UpdateProgress(const FMSMissionProgressUIData& Data);
	void SetBossHpProgress(float InNormalized);
	// 보스 HP 텍스트 갱신
	void SetTextBossHp(float CurrentHp, float MaxHp);
	void UpdateFindCount(int32 Current, int32 Target);
	// 일반 미션용 Progress UI 표시
	void ShowDefaultProgress();
	// 보스용 Progress UI 표시
	void ShowBossProgress();
	void ShowFindTargetUI();
	// 미션 타이머 중지
	void StopMissionTimer();
public:
	// 일반 목표 Progress 영역
	UPROPERTY(meta = (BindWidget))
	class USizeBox* SizeBox_Other;
	UPROPERTY(meta = (BindWidget))
	class USizeBox* SizeBox_Boss;
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* Progress_BossHp;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_BossHp;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_Timer;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_MissionTitle;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_MissionMessage;
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* Progress_TargetHp;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_FindTarget;

	FTimerHandle UITimerHandle;
	float MissionEndTime;
	UPROPERTY()
	TObjectPtr<AMSGameState> GameState;
};
