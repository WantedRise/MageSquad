// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "MSGameState.generated.h"


DECLARE_MULTICAST_DELEGATE_OneParam(FOnProgressUpdated, float/* Normalized */);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMissionChanged, int32/* MissionID */);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMissionProgressChanged, float);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnMissionFinished, int32/* MissionID */,bool/* Result*/ );


// 현재 레벨 내 공유 경험치 값 변동 델리게이트
DECLARE_MULTICAST_DELEGATE(FOnSharedExperienceChangedNative);

// 레벨업 발생 델리게이트
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSharedLevelUpNative, int32 /*NewLevel*/);

// 플레이 인원 변동 델리게이트
DECLARE_MULTICAST_DELEGATE_OneParam(FOnActivePlayerCountChangedNative, int32 /*NewCount*/);

/*
* 작성자: 이상준
* 작성일: 2025-12-20
* 인게임 전용 GameState
* - GameFlow와 ProgressComponent를 소유하는 상위 허브 역할
* - 동기화를 위해 게임 진행률, 보스 상태, 흐름 시작/종료를 중앙에서 제어
*
* 수정자: 김준형
* 수정일: 25/12/22
* 공유 경험치/레벨 시스템 추가
* - 경험치/레벨은 모든 플레이어가 공유 (누가 먹어도 모두 동일하게 증가)
* - 플레이 인원(1~4)에 따라 레벨별 필요 경험치가 달라지며, 인원 변동 시 유동적으로 재계산
* 
* 수정자: 박세찬
* 수정일: 25/12/22
* 레벨업 시 스킬 레벨업 선택지 호출 함수 추가
*/
UCLASS()
class MAGESQUAD_API AMSGameState : public AGameState
{
	GENERATED_BODY()
public:
	AMSGameState();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
	virtual void HandleMatchIsWaitingToStart() override;

	// AGameStateBase: 서버에서 PlayerState가 추가/제거될 때 호출됨
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

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
	float GetServerTime() const { return CurrentServerTime; }
	float GetMissionEndTime() { return MissionEndTime; }
public:
	/* ===== Server Only ===== */
	//현재 게임 진행률(0~1)을 설정
	void SetProgressNormalized(float InPercent, float InServerTime) { ProgressNormalized = InPercent; CurrentServerTime = InServerTime;  };
	//현재 미션ID를 설정
	UFUNCTION()
	void SetCurrentMissionID(int32 InMissionID);
	//미션 진행도 설정
	void SetMissionProgress(float NewProgress);
	//미션 완료 알림
	void NotifyMissionFinished(bool bSuccess);
	//
	void SetMissionEndTime(float InMissionEndTime) { MissionEndTime = InMissionEndTime; }
protected:
	/* ===== Replicated State ===== */
	//현재 미션ID
	UPROPERTY(ReplicatedUsing = OnRep_CurrentMissionID)
	int32 CurrentMissionID = INDEX_NONE;
	//미션 진행도
	UPROPERTY(ReplicatedUsing = OnRep_MissionProgress)
	float MissionProgress = 0.f;
	//끝난 미션 카운트
	UPROPERTY(ReplicatedUsing = OnRep_MissionFinished)
	uint8 MissionFinishedCounter = 0;
	//미션 성공 여부
	UPROPERTY(Replicated)
	uint8 bMissionSuccess : 1;
	//전체 게임 진행도를 나타내는 정규화된 값
	UPROPERTY(ReplicatedUsing = OnRep_ProgressNormalized)
	float ProgressNormalized;
	UPROPERTY(Replicated)
	float CurrentServerTime;
	UPROPERTY(Replicated)
	float MissionEndTime = 0.f;
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



	/*****************************************************
	* Shared Experience & Level Section
	*****************************************************/
public:
	// 공유 경험치 누적 함수 (서버 전용)
	UFUNCTION(BlueprintCallable, Category = "Custom | Experience")
	void AddSharedExperience_Server(AActor* SourceActor, float BaseXP);

	// 현재 공유 레벨 반환 함수
	UFUNCTION(BlueprintPure, Category = "Custom | Experience")
	int32 GetSharedLevel() const { return SharedLevel; }

	// 현재 레벨 내 공유 경험치 반환 함수
	UFUNCTION(BlueprintPure, Category = "Custom | Experience")
	float GetSharedCurrentXP() const { return SharedCurrentXP; }

	// 현재 레벨 요구 경험치 반환 함수
	UFUNCTION(BlueprintPure, Category = "Custom | Experience")
	float GetSharedXPRequired() const { return SharedXPRequired; }

	// 현재 경험치 진행도 반환 함수 (0 ~ 1)
	UFUNCTION(BlueprintPure, Category = "Custom | Experience")
	float GetSharedXPPct() const;

	// 현재 플레이 인원 반환 함수 (1~4인)
	UFUNCTION(BlueprintPure, Category = "Custom | Experience")
	int32 GetActivePlayerCount() const { return ActivePlayerCount; }

	// 스킬 레벨업 선택지 Phase 시작 (서버 전용)
	UFUNCTION(BlueprintCallable)
	void StartSkillLevelUpPhase();
	
	void NotifySkillLevelUpCompleted(class AMSPlayerState* PS);
	
	float GetSkillLevelUpRemainingSeconds_Server() const;
private:
	bool bSkillLevelUpPhaseActive = false;
	int32 CurrentSkillLevelUpSessionId = 0;
	double SkillLevelUpExpireAtRealTime = 0.0;

	FTimerHandle SkillLevelUpPollTimer;

	UPROPERTY()
	TSet<TObjectPtr<class AMSPlayerState>> CompletedPlayers;

	void PollSkillLevelUpPhase();
	void EndSkillLevelUpPhase(bool bByTimeout);
	bool AreAllPlayersCompleted() const;
	
	
protected:
	// 현재 레벨 변동 OnRep 함수
	UFUNCTION()
	void OnRep_SharedLevel();

	// 현재 레벨 내 공유 경험치 변동 OnRep 함수
	UFUNCTION()
	void OnRep_SharedExperience();

	// 현재 플레이 인원 변동 OnRep 함수
	UFUNCTION()
	void OnRep_ActivePlayerCount();

	

private:
	// 서버: PlayerArray 기반 유효 인원 계산 함수
	int32 ComputeActivePlayerCount_Server() const;

	// 서버: 현재 레벨의 요구 경험치 재계산 함수
	void RecalculateRequiredXP_Server(bool bPreservePct);

	// 서버: 레벨업 처리 함수
	void ProcessLevelUps_Server();

	// 서버: 모든 클라이언트에게 레벨업 효과를 처리하도록 알리는 함수
	void BroadcastSharedLevelUp_ServerOnly();

	// 서버: 레벨별 요구 경험치 조회 함수
	float GetRequiredXPForLevel_Server(int32 Level, int32 PlayerCount) const;

	// 서버: 경헙치 획득자의 개인 보정치(ExperienceGainMod) 반영 함수
	float ApplyPersonalExperienceModifiers_Server(AActor* SourceActor, float BaseXP) const;

	// 현재 레벨 내 공유 경험치 변동 델리게이트 콜백 함수
	void BroadcastExperienceChanged();

	// 현재 플레이 인원 변동 콜백 함수
	void BroadcastPlayerCountChanged();

public:
	// 현재 레벨 내 공유 경험치 값 변동 델리게이트 
	FOnSharedExperienceChangedNative OnSharedExperienceChanged;

	// 레벨업 발생 델리게이트 (서버에서만 LevelUp 횟수 확정, 클라는 최종 레벨만 전달)
	FOnSharedLevelUpNative OnSharedLevelUp;

	// 플레이 인원 변동 델리게이트
	FOnActivePlayerCountChangedNative OnActivePlayerCountChanged;

protected:
	// 현재 공유 레벨
	UPROPERTY(ReplicatedUsing = OnRep_SharedLevel, VisibleInstanceOnly, Category = "Custom | Experience")
	int32 SharedLevel = 1;

	// 현재 레벨 내 공유 경험치
	UPROPERTY(ReplicatedUsing = OnRep_SharedExperience, VisibleInstanceOnly, Category = "Custom | Experience")
	float SharedCurrentXP = 0.f;

	// 현재 레벨 요구 경험치 (서버에서 계산 후 복제)
	UPROPERTY(ReplicatedUsing = OnRep_SharedExperience, VisibleInstanceOnly, Category = "Custom | Experience")
	float SharedXPRequired = 100.f;

	// 레벨별 필요 경험치 데이터 (서버에서만 필요. 클라는 RepRequired 사용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Experience")
	TObjectPtr<UDataTable> LevelExpTable = nullptr;

	// 플레이 인원 변동 시, 현재 퍼센트 유지 옵션
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Experience")
	bool bPreserveProgressPctOnPlayerCountChange = true;

	// 현재 플레이 인원(1~4인, 서버 계산 후 복제)
	UPROPERTY(ReplicatedUsing = OnRep_ActivePlayerCount, VisibleInstanceOnly, Category = "Custom | Experience")
	int32 ActivePlayerCount = 1;
};
