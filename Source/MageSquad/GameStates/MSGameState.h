// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "MSGameState.generated.h"


DECLARE_MULTICAST_DELEGATE_OneParam(FOnProgressUpdated, float/* Normalized */);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMissionChanged, int32/* MissionID */);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMissionProgressChanged, float);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMissionFinished, bool);

/*
* 작성자: 이상준
* 작성일: 2025-12-20
* 인게임 전용 GameState
* - GameFlow와 ProgressComponent를 소유하는 상위 허브 역할
* - 동기화를 위해 게임 진행률, 보스 상태, 흐름 시작/종료를 중앙에서 제어
*
*/
UCLASS()
class MAGESQUAD_API AMSGameState : public AGameState
{
	GENERATED_BODY()
public:
	AMSGameState();
	virtual void BeginPlay() override;
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
	virtual void HandleMatchIsWaitingToStart() override;
public:
	//GameFlow 인스턴스 생성 및 초기 설정
	void SetupGameFlow();

	//게임 진행률 변경 시 알림 델리게이트
	FOnProgressUpdated OnProgressUpdated;
	//미션 변경시 알림 델리게이트
	FOnMissionChanged OnMissionChanged;
	//미션 진행 상황 변동 알림 델리게이트
	FOnMissionProgressChanged OnMissionProgressChanged;
	//미션 종료 알림 델리게이트
	FOnMissionFinished OnMissionFinished;
public:
	//OnProgressUpdated 델리게이트 브로드캐스트
	UFUNCTION()
	void OnRep_ProgressNormalized();
	//OnMissionChanged 델리게이트 브로드캐스트
	UFUNCTION()
	void OnRep_CurrentMissionID();
	UFUNCTION()
	void OnRep_MissionProgress();
	UFUNCTION()
	void OnRep_MissionFinished();
	//현재 게임 진행률 반환
	float GetProgressNormalized() const { return ProgressNormalized; };
public:
	/* ===== Server Only ===== */
	//현재 게임 진행률(0~1)을 설정
	void SetProgressNormalized(float InPercent) { ProgressNormalized = InPercent; };
	//현재 미션ID를 설정
	void SetCurrentMissionID(int32 InMissionID);
	//미션 진행도 설정
	void SetMissionProgress(float NewProgress);
	//미션 완료 알림
	void NotifyMissionFinished(bool bSuccess);
protected:
	/* ===== Replicated State ===== */
	//현재 미션ID
	UPROPERTY(ReplicatedUsing = OnRep_CurrentMissionID)
	int32 CurrentMissionID = INDEX_NONE;
	//미션 진행도
	UPROPERTY(ReplicatedUsing = OnRep_MissionProgress)
	float MissionProgress = 0.f;
	//미션 성공 여부
	UPROPERTY(ReplicatedUsing = OnRep_MissionFinished)
	uint8 bMissionSuccess : 1;
	//전체 게임 진행도를 나타내는 정규화된 값
	UPROPERTY(ReplicatedUsing = OnRep_ProgressNormalized)
	float ProgressNormalized;
protected:

	//UPROPERTY(ReplicatedUsing = OnRep_RemainingTime)
	//float RemainingMissionTime;
	//
	UPROPERTY(VisibleAnywhere)
	class UMSMissionComponent* MissionComponent;
	UPROPERTY(VisibleAnywhere)
	class UMSGameProgressComponent* GameProgress;
	//현재 미션을 관리하는 컴포넌트
	//UPROPERTY(VisibleAnywhere)
	//class UMSMission* GameProgress;
	//현재 게임의 흐름을 정의하는 GameFlow 인스턴스
};
