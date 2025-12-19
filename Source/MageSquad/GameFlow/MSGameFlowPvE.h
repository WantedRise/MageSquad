// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MSGameFlowBase.h"
#include "MSGameFlowPvE.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class MAGESQUAD_API UMSGameFlowPvE : public UMSGameFlowBase
{
	GENERATED_BODY()

public:
	virtual void Initialize(class AMSGameState* InOwnerGameState) override;
	virtual void TickFlow(float DeltaSeconds) override;
	virtual void OnEnterState(EGameFlowState NewState) override;
	virtual void OnExitState(EGameFlowState PreState) override;
	virtual void Start() override;
	void RegisterTimeEvent(EGameFlowState EventType, float TriggerTime);

	void OnGameEvent(EGameFlowState InEventType);
	//미션컴포넌트 OnMissionFinished에 바인드할 함수
	void HandleMissionFinished(int32 MissionId, bool bSuccess);

private:
	UFUNCTION()
	void OnTimeCheckpoint();
	int32 SelectRandomMissionId();
	void TriggerRandomMission();

private:
	//PvE 튜닝 값(나중에 DataAsset로 빼도 됨)
	UPROPERTY(EditDefaultsOnly, Category = "GameFlow|PvE")
	float TimeToEnterQuesting = 30.f;

	UPROPERTY(EditDefaultsOnly, Category = "GameFlow|PvE")
	float TimeToSpawnBoss = 300.f;
	//보스 처치 여부
	uint8 bIsBossDefeated = false;
	//이벤트를 관리하기 위해
	UPROPERTY()
	TArray<FTimerHandle> ScheduledEventHandles;
};