// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/MSGameProgressComponent.h"
#include "GameStates/MSGameState.h"

// Sets default values for this component's properties
UMSGameProgressComponent::UMSGameProgressComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

// Called when the game starts
void UMSGameProgressComponent::BeginPlay()
{
	Super::BeginPlay();

    OwnerGameState = GetOwner<AMSGameState>();
}
//이벤트 시간, 이벤트 텍스쳐 전달
void UMSGameProgressComponent::InitProgress()
{
}

void UMSGameProgressComponent::StartProgress()
{
    GameStartTime = GetWorld()->GetTimeSeconds();
    bRunning = true;

    // 예시: 1, 4, 7, 10분 이벤트
    TimeCheckpoints = { 60.f, 240.f, 420.f, 600.f };
    NextCheckpointIndex = 0;

    GetWorld()->GetTimerManager().SetTimer(
        ProgressTimerHandle,
        this,
        &UMSGameProgressComponent::TickProgress,
        1.f,
        true
    );
}

void UMSGameProgressComponent::StopProgress()
{
}

float UMSGameProgressComponent::GetProgressRatio() const
{
    return 0.0f;
}

void UMSGameProgressComponent::TickProgress()
{
    if (!bRunning || NextCheckpointIndex >= TimeCheckpoints.Num())
        return;

    const float Elapsed = OwnerGameState->GetGameElapsedTime();

    if (Elapsed >= TimeCheckpoints[NextCheckpointIndex])
    {
        OnGameTimeReached.Broadcast(Elapsed);
        NextCheckpointIndex++;
    }
}
