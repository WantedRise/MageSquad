// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MSWaveManager.generated.h"

class AMSWaveObstacleGroup;
class AMSWaveCenterPivot;


UCLASS()
class MAGESQUAD_API AMSWaveManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMSWaveManager();

protected:
	virtual void BeginPlay() override;

	

public:
	/* ===== External ===== */

	// GameFlow에서 호출
	void RequestSpawnWave();
	void StartWaveTimer(float DelayNextWaveTime, float EndTime);
	
	void StopWaveTimer();
protected:
	/* ===== Internal ===== */
	void ScheduleNextWave();
	void HandleWaveFinished();
	void SpawnPivotIfNeeded();
	void SpawnWaveInternal();
	FVector CalculateInitialWaveLocation() const;

protected:
	/* ===== Classes ===== */

	UPROPERTY(EditAnywhere, Category = "Wave")
	TSubclassOf<AMSWaveCenterPivot> PivotClass;

	UPROPERTY(EditAnywhere, Category = "Wave")
	TSubclassOf<AMSWaveObstacleGroup> WaveClass;

protected:
	/* ===== Settings ===== */

	// X 시작 오프셋 (큰 X)
	UPROPERTY(EditAnywhere, Category = "Wave")
	float WaveStartXOffset = 3000.f;

	// 이동 거리
	UPROPERTY(EditAnywhere, Category = "Wave")
	float WaveMoveDistance = 6000.f;

	// 회전 범위
	UPROPERTY(EditAnywhere, Category = "Wave")
	float MinYaw = -30.f;

	UPROPERTY(EditAnywhere, Category = "Wave")
	float MaxYaw = 30.f;

	float DelayAfterWave;
protected:
	/* ===== Runtime ===== */

	UPROPERTY()
	AMSWaveCenterPivot* MapPivot;

	UPROPERTY()
	AMSWaveObstacleGroup* ActiveWave;

private:
	FTimerHandle WaveEndTimerHandle;
	FTimerHandle NextWaveTimerHandle;
	FVector StartLocation;
	bool bWaveRunning = false;
};