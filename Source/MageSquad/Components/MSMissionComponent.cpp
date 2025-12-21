// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/MSMissionComponent.h"
#include "DataStructs/MSGameMissionData.h"
#include "GameStates/MSGameState.h"

// Sets default values for this component's properties
UMSMissionComponent::UMSMissionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UMSMissionComponent::BeginPlay()
{
	Super::BeginPlay();

    OwnerGameState = GetOwner<AMSGameState>();
	// ...
    if (!IsServer())
    {
        BindGameStateDelegates();
    }
}

void UMSMissionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (IsServer() && MissionScript)
    {
        MissionScript->Deinitialize();
        MissionScript = nullptr;
    }

    Super::EndPlay(EndPlayReason);
}

void UMSMissionComponent::StartMission(const FMSMissionRow& MissionRow)
{
    if (!IsServer())
        return;

    check(MissionRow.ScriptClass);

    if (MissionScript)
    {
        MissionScript->Deinitialize();
        MissionScript = nullptr;
    }

    MissionScript = NewObject<UMSMissionScript>(this,MissionRow.ScriptClass);
    MissionScript->Initialize(GetWorld());
}

void UMSMissionComponent::UpdateMission()
{
    if (!IsServer() || !MissionScript || !OwnerGameState)
        return;

    const float Progress = MissionScript->GetProgress();

    OwnerGameState->SetMissionProgress(Progress);

    if (MissionScript->IsCompleted())
    {
        OwnerGameState->NotifyMissionFinished(true);

        MissionScript->Deinitialize();
        MissionScript = nullptr;
    }
}

void UMSMissionComponent::AbortMission()
{
    if (!IsServer() || OwnerGameState)
        return;

    if (MissionScript)
    {
        MissionScript->Deinitialize();
        MissionScript = nullptr;
    }

    OwnerGameState->NotifyMissionFinished(false);
}

void UMSMissionComponent::BindGameStateDelegates()
{
    if (!OwnerGameState)
        return;

    OwnerGameState->OnMissionChanged.AddUObject(
        this,
        &UMSMissionComponent::HandleMissionChanged
    );

    OwnerGameState->OnMissionProgressChanged.AddUObject(
        this,
        &UMSMissionComponent::HandleMissionProgressChanged
    );

    OwnerGameState->OnMissionFinished.AddUObject(
        this,
        &UMSMissionComponent::HandleMissionFinished
    );
}

void UMSMissionComponent::HandleMissionChanged(int32 MissionID)
{
    // Client:
    // - MissionID → DataSubsystem 조회
    // - UI 표시 / 연출
    
}

void UMSMissionComponent::HandleMissionProgressChanged(float Progress)
{
    // Client:
    // - ProgressBar 갱신

}

void UMSMissionComponent::HandleMissionFinished(bool bSuccess)
{
    // Client:
    // - 성공 / 실패 연출

}

bool UMSMissionComponent::IsServer() const
{
    return GetOwner() && GetOwner()->HasAuthority();
}