// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MSGameFlowBase.h"
#include "DataStructs/MSMissionTimeline.h"
#include "MSGameFlowPvE.generated.h"

/*
* 작성자: 이상준
* 작성일: 2025-12-19
*
* PvE 전용 게임 진행 흐름(Game Flow)을 정의하는 클래스.
*  
*  - 시간 경과에 따른 상태 전환 (예: 플레이 -> 미션 -> 플레이 -> 보스 등장)
*  - 미션 완료 이벤트 수신 및 결과에 따른 흐름 제어
*  - GameFlowBase를 상속하여 PvP / PvE / Coop 등 확장 가능 구조 유지
*  - GameState에 귀속되어 서버 권한 하에서만 동작 (서버에서만 존재)
*
*/
UCLASS(Blueprintable)
class MAGESQUAD_API UMSGameFlowPvE : public UMSGameFlowBase
{
	GENERATED_BODY()

public:
	//PvE 전용 이벤트 및 초기 상태 설정
	virtual void Initialize(class AMSGameState* InOwnerGameState, UDataTable* InTimelineTable) override;
	//미션 시작, 보스 준비 등 트리거 처리
	virtual void OnEnterState(EGameFlowState NewState) override;
	//상태 종료 시 필요한 후처리
	virtual void OnExitState(EGameFlowState PreState) override;
	//게임 흐름을 실제로 시작시키는 진입점
	virtual void Start() override;
	//
	void ScheduleMission(float TriggerTime, int32 MissionID);
	//미션컴포넌트 OnMissionFinished에 바인드할 함수
	void HandleMissionFinished(int32 MissionId, bool bSuccess);

private:
	UFUNCTION()
	void OnTimeCheckpoint();
	int32 SelectRandomMissionId();
	void TriggerRandomMission();
private:
	//이벤트를 관리하기 위해
	UPROPERTY()
	TArray<FTimerHandle> MissionTimerHandles;
	TArray<const FMissionTimelineRow*> MissionTimelineRows;
	int32 CurrentMissionIndex = 0;
	float MissionTriggerTime = 0;
};