// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DataStructs/MSGameMissionData.h"
#include "DataStructs/MSMissionProgressUIData.h"
#include "MSMissionComponent.generated.h"

struct FMSMissionRow;
struct FMSMissionProgressUIData;
struct FMSMissionCleanData;

/*
* 작성자: 이상준
* 작성일: 2025-12-23
*
* 미션의 실행과 수명 주기를 관리하는 서버 중심 ActorComponent
*
* 서버 권한 하에서 미션을 시작 / 갱신 / 종료
* MissionScript를 생성·관리하며 실제 미션 로직을 위임
* GameState에 미션 진행 정보(진행도, 종료 여부)를 전달
*
*/

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MAGESQUAD_API UMSMissionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMSMissionComponent();
public:
	// GameState 참조 획득 및 이벤트 바인딩
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
public: /* ===== Server Only ===== */
	// 새로운 미션 시작
	void StartMission(const FMSMissionRow& MissionRow);
	// MissionScript의 진행도를 GameState에 반영
	void UpdateMission();
	// 강제 종료 (실패)
	void AbortMission();

	FMSMissionRow GetCurrentMissionData() const {
		return CurrentMissionData;
	}
private: 
	/* ===== Server Only ===== */
	// GameState 미션 관련 델리게이트 바인딩
	void BindGameStateDelegates();
	// 제한 시간 만료 시 자동 실패 처리
	void OnMissionTimeExpired();
	// 미션 ID 변경 이벤트 처리, 서버: 실제 미션 시작
	void HandleMissionChanged(int32 MissionID);
	// 미션 종료 공통 처리
	void FinishMission(bool bSuccess);
	void RequestMissionScriptDestroy();
	/* ====================== */
	
	void OnMissionFinished(int32 MissionID,bool bSuccess);
private:
	// 서버 권한 확인
	bool IsServer() const;
private:
	// 서버 전용, 현재 실행 중인 미션 로직 스크립트
	UPROPERTY()
	class UMSMissionScript* MissionScript = nullptr;

	// 소유 GameState
	class AMSGameState* OwnerGameState;
	// 미션 제한 시간 타이머 
	FTimerHandle MissionTimerHandle;

	FMSMissionRow CurrentMissionData;
	FMSMissionProgressUIData CurrentProgress;
};
