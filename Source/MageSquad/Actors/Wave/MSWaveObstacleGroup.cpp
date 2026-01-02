// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Wave/MSWaveObstacleGroup.h"
#include "MSWaveBlock.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

AMSWaveObstacleGroup::AMSWaveObstacleGroup()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostPhysics;

	bReplicates = true;
	SetReplicateMovement(false);

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AMSWaveObstacleGroup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMSWaveObstacleGroup, ServerLocation);
	DOREPLIFETIME(AMSWaveObstacleGroup, bMoving);
}

void AMSWaveObstacleGroup::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Error, TEXT("WaveManager->AMSWaveObstacleGroup"));
	if (!HasAuthority())
	{
		return;
	}

	SpawnBlocksByPattern();


	TArray<USkeletalMeshComponent*> SkeletalMeshes;
	GetComponents<USkeletalMeshComponent>(SkeletalMeshes);

	for (USkeletalMeshComponent* MeshComp : SkeletalMeshes)
	{
		if (!MeshComp)
			continue;

		UE_LOG(LogTemp, Log, TEXT("Found SkeletalMesh: %s"), *MeshComp->GetName());

		const float RandomRate = FMath::FRandRange(0.8f, 1.2f);
		MeshComp->SetPlayRate(RandomRate);
	}

	
}

void AMSWaveObstacleGroup::OnRep_ServerLocation()
{
	TargetLocation = ServerLocation;
}

void AMSWaveObstacleGroup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority() && bMoving)
	{
		const FVector MoveDir = GetActorForwardVector();
		const float DeltaDist = MoveSpeed * DeltaTime;

		AddActorWorldOffset(MoveDir * DeltaDist, false);

		ServerLocation = GetActorLocation();

		MovedDistance += DeltaDist;
		if (MovedDistance >= MaxMoveDistance)
		{
			FinishWave();
			MovedDistance = 0.0f;
		}
	}
	else if (bMoving)
	{
		// ⭐ 클라 보간
		const FVector Smoothed = FMath::VInterpTo(GetActorLocation(), TargetLocation, DeltaTime, 20.f);
		SetActorLocation(Smoothed);
	}
}

void AMSWaveObstacleGroup::SetMoveDistance(float InDistance)
{
	MaxMoveDistance = InDistance;
}

void AMSWaveObstacleGroup::ActivateWave(FVector InStartLocation)
{
	if (!HasAuthority())
	{
		return;
	}
	SetActorLocation(InStartLocation);
	ServerLocation = GetActorLocation();
	UE_LOG(LogTemp, Error, TEXT("WaveManager->ActivateWave"));
	// Block 활성화
	for (AMSWaveBlock* Block : SpawnedBlocks)
	{
		if (Block)
		{
			Block->ActivateBlock();
		}
	}

	SetActorHiddenInGame(false);

	StartMove();
}

void AMSWaveObstacleGroup::DeactivateWave()
{
	if (!HasAuthority())
	{
		return;
	}

	// Block 활성화
	for (AMSWaveBlock* Block : SpawnedBlocks)
	{
		if (Block)
		{
			Block->DeactivateBlock();
		}
	}

	SetActorHiddenInGame(true);

	bMoving = false;
}

void AMSWaveObstacleGroup::SpawnBlocksByPattern()
{
	if (!BlockClass || Pattern.IsEmpty())
	{
		return;
	}

	float Cursor = 0.f;
	int32 PatternIndex = 0;
	const int32 PatternLen = Pattern.Len();

	const FVector Right = GetActorRightVector();

	while (Cursor < TargetWaveWidth)
	{
		const TCHAR Token = Pattern[PatternIndex];

		const bool bIsBlock = (Token == TEXT('B') || Token == TEXT('1'));
		const bool bIsGap = (Token == TEXT('G') || Token == TEXT('0'));

		if (bIsBlock)
		{
			// ⭐ 중앙 기준 연속 배치
			const float CenterOffset =-TargetWaveWidth + Cursor * 2;

			const FVector SpawnLoc =
				GetActorLocation()
				+ Right * CenterOffset;

			AMSWaveBlock* Block =
				GetWorld()->SpawnActor<AMSWaveBlock>(
					BlockClass,
					SpawnLoc,
					GetActorRotation()
				);

			if (Block)
			{
				Block->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
				Block->DeactivateBlock();
				SpawnedBlocks.Add(Block);
			}

			Cursor += BlockLength;
		}
		else if (bIsGap)
		{
			Cursor += GapLength;
		}

		PatternIndex = (PatternIndex + 1) % PatternLen;
	}
}


void AMSWaveObstacleGroup::StartMove()
{
	bMoving = true;
}

void AMSWaveObstacleGroup::FinishWave()
{
	DeactivateWave();

	OnWaveFinished.Broadcast();
}

void AMSWaveObstacleGroup::ApplyWaveRotation(float Yaw)
{
	if (!HasAuthority())
	{
		return;
	}

	MulticastSetWaveRotation(Yaw);
}

void AMSWaveObstacleGroup::MulticastSetWaveRotation_Implementation(float Yaw)
{
	const FRotator NewRot(0.f, Yaw, 0.f);
	SetActorRotation(NewRot);
}