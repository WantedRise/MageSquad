// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Wave/MSWaveManager.h"
#include "MSWaveCenterPivot.h"
#include "MSWaveObstacleGroup.h"

AMSWaveManager::AMSWaveManager()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = false;          // ⭐ 서버 전용
	SetReplicateMovement(false);
}

void AMSWaveManager::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}

	SpawnPivotIfNeeded();
}

void AMSWaveManager::StartWaveTimer(float DelayNextWaveTime,float EndTime)
{
	DelayAfterWave = DelayNextWaveTime;
	bWaveRunning = true;

	// Todo : 보스 연출 끝났을때 멈췄다가 움직여야함
	//GetWorld()->GetTimerManager().SetTimer(
	//	WaveEndTimerHandle,
	//	this,
	//	&AMSWaveManager::StopWaveTimer,
	//	EndTime,
	//	false
	//);

	RequestSpawnWave();
}

void AMSWaveManager::ScheduleNextWave()
{
	if (!bWaveRunning)
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(NextWaveTimerHandle);

	if (DelayAfterWave < 0.1f)
	{
		RequestSpawnWave();
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(
			NextWaveTimerHandle,
			this,
			&AMSWaveManager::RequestSpawnWave,
			DelayAfterWave,
			false
		);
	}
}

void AMSWaveManager::StopWaveTimer()
{
	bWaveRunning = false;

	GetWorld()->GetTimerManager().ClearTimer(NextWaveTimerHandle);

	if (ActiveWave)
	{
		ActiveWave->Destroy();
		ActiveWave = nullptr;
	}

	UE_LOG(LogTemp, Log, TEXT("[WaveManager] Wave system stopped"));
}

void AMSWaveManager::RequestSpawnWave()
{
	SpawnPivotIfNeeded();

	if (ActiveWave)
	{
		// 랜덤 회전
		const float RandomYaw = FMath::FRandRange(MinYaw, MaxYaw);
		MapPivot->SetActorRotation(FRotator(0.f, RandomYaw, 0.f));
		// 클라 동기화
		ActiveWave->ApplyWaveRotation(ActiveWave->GetActorRotation().Yaw);
		ActiveWave->ActivateWave(StartLocation);
		return;
	}

	SpawnWaveInternal();
}

void AMSWaveManager::SpawnPivotIfNeeded()
{
	if (MapPivot)
	{
		return;
	}

	if (!PivotClass)
	{
		return;
	}

	// 맵 중앙 (고정)
	MapPivot = GetWorld()->SpawnActor<AMSWaveCenterPivot>(
		PivotClass,
		FVector::ZeroVector,
		FRotator::ZeroRotator
	);

	UE_LOG(LogTemp, Error, TEXT("WaveManager->MapPivot"));
}

void AMSWaveManager::SetWavePaused(bool bIsStartCutscene)
{
	if (!ActiveWave)
	{
		return;
	}

	if (bIsStartCutscene)
	{
		GetWorld()->GetTimerManager().PauseTimer(NextWaveTimerHandle);
		ActiveWave->bMoving = false;
	}
	else
	{
		GetWorld()->GetTimerManager().UnPauseTimer(NextWaveTimerHandle);
		ActiveWave->bMoving = true;
	}
}

void AMSWaveManager::SpawnWaveInternal()
{
	if (!WaveClass || !MapPivot)
	{
		return;
	}

	StartLocation = CalculateInitialWaveLocation();

	ActiveWave = GetWorld()->SpawnActor<AMSWaveObstacleGroup>(
		WaveClass,
		StartLocation,
		FRotator(0, 180.0f,0.f)
	);
	
	ActiveWave->OnWaveFinished.AddUObject(
		this,
		&AMSWaveManager::HandleWaveFinished
	);

	if (!ActiveWave)
	{
		return;
	}

	// Pivot에 부착 (회전 기준)
	ActiveWave->AttachToActor(
		MapPivot,
		FAttachmentTransformRules::KeepRelativeTransform
	);

	StartLocation.Z += ActiveWave->DefaultHeight;
	ActiveWave->SetActorLocation(StartLocation);

	// 랜덤 회전
	const float RandomYaw = FMath::FRandRange(MinYaw, MaxYaw);
	MapPivot->SetActorRotation(FRotator(0.f, RandomYaw, 0.f));

	// 이동 거리 전달
	ActiveWave->SetMoveDistance(WaveMoveDistance);

	// 웨이브 시작
	ActiveWave->ActivateWave(StartLocation);
}

void AMSWaveManager::HandleWaveFinished()
{
	if (!HasAuthority())
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[WaveManager] Wave finished"));

	ScheduleNextWave();
	// 여기서 다음 웨이브 대기 / 카운트 / 난이도 증가 등
}

FVector AMSWaveManager::CalculateInitialWaveLocation() const
{
	if (!MapPivot)
	{
		return FVector::ZeroVector;
	}

	// X값이 큰 위치에서 시작
	return MapPivot->GetActorLocation()
		+ FVector(WaveStartXOffset, 0.f, 0.f);
}
