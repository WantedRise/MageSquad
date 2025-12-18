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
void UMSGameProgressComponent::Initialize(float InTotalGameTime)
{
    TotalGameTime = InTotalGameTime;
}

void UMSGameProgressComponent::StartProgress()
{
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
    GetWorld()->GetTimerManager().ClearTimer(ProgressTimerHandle);
}

float UMSGameProgressComponent::GetNormalizedProgress() const
{
    if (!OwnerGameState)
    {
        return 0.f;
    }

    return FMath::Clamp(
        OwnerGameState->GetElapsedGameTime() / TotalGameTime,
        0.f,
        1.f
    );
}

void UMSGameProgressComponent::TickProgress()
{
    if (!bRunning || NextCheckpointIndex >= TimeCheckpoints.Num())
        return;
    OwnerGameState->AddElapsedGameTime(1.f);
}
