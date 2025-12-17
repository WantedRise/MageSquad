// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MSGameFlowBase.h"
#include "MSGameFlowPvE.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UMSGameFlowPvE : public UMSGameFlowBase
{
	GENERATED_BODY()

public:
	virtual void Initialize(class AMSGameState* InOwnerGameState);
	virtual void TickFlow(float DeltaSeconds) override;

	void RegisterTimeEvent(EGameFlowState EventType, float TriggerTime);

	void OnGameEvent(EGameFlowState InEventType);
	//미션컴포넌트 OnMissionFinished에 바인드할 함수
	void HandleMissionFinished(int32 MissionId, bool bSuccess);
private:
	int32 SelectRandomMissionId();
	void TriggerRandomMission();

	void EnterNone();
	void EnterPlaying();
	void EnterQuesting();
	void EnterBoss();

private:
	// PvE 튜닝 값(나중에 DataAsset로 빼도 됨)
	UPROPERTY(EditDefaultsOnly, Category = "GameFlow|PvE")
	float TimeToEnterQuesting = 30.f;

	UPROPERTY(EditDefaultsOnly, Category = "GameFlow|PvE")
	float TimeToSpawnBoss = 300.f;
	// 내부 플래그(최소)
	uint8 bIsBossDefeated = false;

	UPROPERTY()
	TArray<FTimerHandle> ScheduledEventHandles;
};