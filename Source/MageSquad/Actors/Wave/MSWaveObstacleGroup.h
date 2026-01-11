// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MSWaveObstacleGroup.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnWaveFinished);

class AMSWaveBlock;
class AMSWaveCenterPivot;

UCLASS()
class MAGESQUAD_API AMSWaveObstacleGroup : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMSWaveObstacleGroup();
protected:
	virtual void BeginPlay() override;
	UFUNCTION()
	void OnRep_ServerLocation();
	virtual void Tick(float DeltaTime) override;
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetWaveRotation(float Yaw);
public:
	/* ===== Settings ===== */
	FOnWaveFinished OnWaveFinished;
	void ApplyWaveRotation(float Yaw);
	// Block 길이 (Y 방향)
	UPROPERTY(EditAnywhere, Category = "Wave")
	float BlockLength = 930.f;

	// Gap 길이
	UPROPERTY(EditAnywhere, Category = "Wave")
	float GapLength = 200.f;

	UPROPERTY(EditAnywhere, Category = "Wave")
	float DefaultHeight = 90.0f;

	// 좌우 전체 폭 (패턴 반복 기준)
	UPROPERTY(EditAnywhere, Category = "Wave")
	float TargetWaveWidth = 3500.f;

	// 이동 거리 (X 감소)
	UPROPERTY(EditAnywhere, Category = "Wave")
	float MovedDistance = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Wave|Move")
	float MaxMoveDistance = 6000.f;

	UPROPERTY(EditAnywhere, Category = "Wave")
	float MoveSpeed = 600.0f;

	UPROPERTY(Replicated)
	uint8 bMoving : 1;
	
	// Block 클래스
	UPROPERTY(EditAnywhere, Category = "Wave")
	TSubclassOf<AMSWaveBlock> BlockClass;

	/**
	 * 패턴 문자열
	 * 'B' or '1' = Block
	 * 'G' or '0' = Gap
	 */
	UPROPERTY(EditAnywhere, Category = "Wave")
	FString Pattern;

	UPROPERTY(ReplicatedUsing = OnRep_ServerLocation)
	FVector ServerLocation;
	FVector TargetLocation;
public:
	/* ===== Control ===== */

	void ActivateWave(FVector InStartLocation);
	void DeactivateWave();
	void SetMoveDistance(float InDistance);
	// 달리기 사운드 켜기
	void StartRunSound();
	// 달리기 사운드 끄기
	void StopRunSound();
protected:
	/* ===== Internal ===== */

	void SpawnBlocksByPattern();
	void StartMove();
	void FinishWave();

protected:
	/* ===== Runtime ===== */

	UPROPERTY()
	TArray<AMSWaveBlock*> SpawnedBlocks;

	FVector StartLocation;
	FVector EndLocation;

	float ElapsedTime = 0.f;
	FTimerHandle MoveTimerHandle;

	// 사운드
	UPROPERTY(EditDefaultsOnly, Category = "Custom | SFX", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USoundBase> RunSound;
	UPROPERTY()
	TObjectPtr<class UAudioComponent> RunAudioComponent;
	UPROPERTY(EditDefaultsOnly, Category = "Custom | SFX", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USoundAttenuation> AttenuationSettings;
};