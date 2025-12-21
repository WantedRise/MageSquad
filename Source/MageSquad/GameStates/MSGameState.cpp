// Fill out your copyright notice in the Description page of Project Settings.


#include "GameStates/MSGameState.h"
#include "GameModes/MSGameMode.h"
#include "GameFlow/MSGameFlowBase.h"
#include "MageSquad.h"
#include "Net/UnrealNetwork.h"
#include "Components/MSGameProgressComponent.h"
#include "Components/MSMissionComponent.h"
AMSGameState::AMSGameState()
{
    MissionComponent = CreateDefaultSubobject<UMSMissionComponent>(TEXT("MissionComponent"));
    //GameProgress = CreateDefaultSubobject<UMSGameProgressComponent>(TEXT("GameProgress"));
}

void AMSGameState::BeginPlay()
{
	Super::BeginPlay();

    
}
void AMSGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMSGameState, ProgressNormalized);
    DOREPLIFETIME(AMSGameState, CurrentMissionID);
    DOREPLIFETIME(AMSGameState, MissionProgress);
    DOREPLIFETIME(AMSGameState, bMissionSuccess);
}

void AMSGameState::HandleMatchIsWaitingToStart()
{
    Super::HandleMatchIsWaitingToStart();
    if (HasAuthority())
    {
        if (nullptr == GameProgress)
        {
            GameProgress = NewObject<UMSGameProgressComponent>(this);
            GameProgress->RegisterComponent();
        }
    }
}

void AMSGameState::SetCurrentMissionID(int32 NewMissionID)
{
    if (!HasAuthority())
        return;

    CurrentMissionID = NewMissionID;
    OnMissionChanged.Broadcast(CurrentMissionID);
}

void AMSGameState::SetMissionProgress(float NewProgress)
{
    if (!HasAuthority())
        return;

    MissionProgress = FMath::Clamp(NewProgress, 0.f, 1.f);
    OnMissionProgressChanged.Broadcast(MissionProgress);
}

void AMSGameState::NotifyMissionFinished(bool bSuccess)
{
    if (!HasAuthority())
        return;

    bMissionSuccess = bSuccess;
    OnMissionFinished.Broadcast(bMissionSuccess);
}

void AMSGameState::OnRep_ProgressNormalized()
{
    OnProgressUpdated.Broadcast(ProgressNormalized);
}

void AMSGameState::OnRep_CurrentMissionID()
{
    OnMissionChanged.Broadcast(CurrentMissionID);
}

void AMSGameState::OnRep_MissionProgress()
{
    OnMissionProgressChanged.Broadcast(MissionProgress);
}

void AMSGameState::OnRep_MissionFinished()
{
    OnMissionFinished.Broadcast(bMissionSuccess);
}