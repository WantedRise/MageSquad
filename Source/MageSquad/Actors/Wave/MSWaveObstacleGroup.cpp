// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Wave/MSWaveObstacleGroup.h"
#include "MSWaveBlock.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include <Kismet/GameplayStatics.h>
#include "Components/AudioComponent.h"

AMSWaveObstacleGroup::AMSWaveObstacleGroup()
{
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	bReplicates = true;
	bAlwaysRelevant = true;
	PrimaryActorTick.bCanEverTick = true;
	SetReplicateMovement(true);
	SetNetCullDistanceSquared(0.0f);
	SetNetUpdateFrequency(100.0f);
	SetMinNetUpdateFrequency(60.f);
	NetPriority = 3.f;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AMSWaveObstacleGroup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMSWaveObstacleGroup, bMoving);
}

void AMSWaveObstacleGroup::BeginPlay()
{
	Super::BeginPlay();

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

void AMSWaveObstacleGroup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority() && bMoving)
	{
		const FVector MoveDir = GetActorForwardVector();
		const float DeltaDist = MoveSpeed * DeltaTime;

		//AddActorWorldOffset(MoveDir * DeltaDist, false);

		const float FixedStep = GetWorld()->GetDeltaSeconds();
		const FVector Velocity = GetActorForwardVector() * MoveSpeed;
		AddActorWorldOffset(Velocity * FixedStep);

		MovedDistance += DeltaDist;
		if (MovedDistance >= MaxMoveDistance)
		{
			FinishWave();
			MovedDistance = 0.0f;
		}
	}
}

void AMSWaveObstacleGroup::OnRep_Moving()
{
	if (bMoving)
	{
		StartRunSound();
	}
	else
	{
		StopRunSound();
	}
}

void AMSWaveObstacleGroup::SetMoveDistance(float InDistance)
{
	MaxMoveDistance = InDistance;
}

//Todo : 비활성화 활성화로 인해 소리 작동이 안됨
void AMSWaveObstacleGroup::StartRunSound()
{
	UE_LOG(LogTemp, Warning, TEXT("RunAudioComponent: StartRunSound"));
	if (!RunSound)
		return;
	UE_LOG(LogTemp, Warning, TEXT("RunAudioComponent: RunSound"));

	if (!RunAudioComponent && AttenuationSettings)
	{
		RunAudioComponent = UGameplayStatics::SpawnSoundAttached(
			RunSound,              // 재생할 사운드 (USoundBase 또는 USoundCue)
			GetRootComponent(),         // 사운드를 첨부할 컴포넌트
			FName(TEXT("")),          // 소켓 이름 (특정 소켓에 첨부하려면 소켓 이름 지정, 없으면 빈 이름)
			FVector(ForceInit),       // 상대 위치 오프셋 (FVector::ZeroVector 사용 가능)
			FRotator(ForceInit),      // 상대 회전 오프셋 (FRotator::ZeroRotator 사용 가능)
			EAttachLocation::SnapToTarget, // 첨부 방식 (타겟 위치/회전에 맞춤)
			true,                     // 자동 활성화 여부
			1.0f,                     // 볼륨 멀티플라이어
			1.0f,                     // 피치 멀티플라이어
			0.0f,                     // 시작 시간 (초)
			AttenuationSettings,                  // 감쇠 설정 (USoundAttenuation* 타입, nullptr 시 애셋 기본값)
			nullptr,                  // 동시성 설정 (USoundConcurrency* 타입, nullptr 시 애셋 기본값)
			false                     // 레벨 전환 시 지속 여부
		);
	}
	else
	{
		RunAudioComponent->Play();
	}

	if (RunAudioComponent && RunAudioComponent->IsPlaying())
	{
		UE_LOG(LogTemp, Warning, TEXT("RunAudioComponent: %s"),
			*GetNameSafe(RunAudioComponent));
	}
}

void AMSWaveObstacleGroup::StopRunSound()
{
	if (!RunAudioComponent)
		return;
	UE_LOG(LogTemp, Warning, TEXT("End RunAudioComponent: %s"),
		*GetNameSafe(RunAudioComponent));
	RunAudioComponent->FadeOut(0.2f, 0.f);   // 자연스럽게 끄기
}

void AMSWaveObstacleGroup::ActivateWave(FVector InStartLocation)
{
	if (!HasAuthority())
	{
		return;
	}
	SetActorLocation(InStartLocation);

	// Block 활성화
	for (AMSWaveBlock* Block : SpawnedBlocks)
	{
		if (Block)
		{
			Block->ActivateBlock();
		}
	}
	MulticastPlaySound();
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
	MulticastStopSound();
	SetActorHiddenInGame(true);

	bMoving = false;
}

void AMSWaveObstacleGroup::SpawnBlocksByPattern()
{
	if (!BlockClass || Pattern.IsEmpty())
		return;

	float Cursor = 0.f;
	int32 PatternIndex = 0;
	const int32 PatternLen = Pattern.Len();

	while (Cursor < TargetWaveWidth)
	{
		const TCHAR Token = Pattern[PatternIndex];

		const bool bIsBlock = (Token == TEXT('B') || Token == TEXT('1'));
		const bool bIsGap = (Token == TEXT('G') || Token == TEXT('0'));

		if (bIsBlock)
		{
			// ⭐ WaveGroup 기준 로컬 X 위치
			const float LocalX = -TargetWaveWidth + Cursor * 2.f;

			// Group 기준 오른쪽 벡터 방향 (Y축)
			const FVector LocalOffset = FVector(0.f, LocalX, 0.f);

			AMSWaveBlock* Block = GetWorld()->SpawnActor<AMSWaveBlock>(
				BlockClass,
				FVector::ZeroVector,
				FRotator::ZeroRotator
			);

			if (Block)
			{
				// ⭐ Group에 로컬 기준으로 Attach
				Block->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);

				// ⭐ 패턴 좌표를 RelativeLocation으로 설정
				Block->SetActorRelativeLocation(LocalOffset);
				Block->SetActorRelativeRotation(FRotator::ZeroRotator);

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

void AMSWaveObstacleGroup::MulticastPlaySound_Implementation()
{
	StartRunSound();
}

void AMSWaveObstacleGroup::MulticastStopSound_Implementation()
{
	StopRunSound();
}