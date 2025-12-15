// Fill out your copyright notice in the Description page of Project Settings.


#include "GameStates/MSLobbyGameState.h"
#include "Net/UnrealNetwork.h"
#include <GameModes/MSLobbyGameMode.h>

void AMSLobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMSLobbyGameState, RemainingReadyTime);

}

void AMSLobbyGameState::StopReadyCountdown()
{
    GetWorld()->GetTimerManager().ClearTimer(ReadyTimerHandle);
}

void AMSLobbyGameState::StartReadyCountdown(int32 StartSeconds)
{
    RemainingReadyTime = StartSeconds;

    if (HasAuthority())
    {
        OnRep_RemainingTime();
    }

    GetWorld()->GetTimerManager().SetTimer(
        ReadyTimerHandle,
        this,
        &AMSLobbyGameState::TickReadyCountdown,
        1.0f,
        true
    );
}

void AMSLobbyGameState::TickReadyCountdown()
{
    if (RemainingReadyTime <= 0)
    {
        GetWorld()->GetTimerManager().ClearTimer(ReadyTimerHandle);
        OnReadyCountdownFinished();
        return;
    }

    --RemainingReadyTime;

    if (HasAuthority())
    {
        OnRep_RemainingTime();
    }
}

void AMSLobbyGameState::OnReadyCountdownFinished()
{
    if (HasAuthority())
    {
        if (AMSLobbyGameMode* GM = GetWorld()->GetAuthGameMode<AMSLobbyGameMode>())
        {
            GM->HandleReadyCountdownFinished();
        }
    }
}

void AMSLobbyGameState::OnRep_LobbyReadyPhase()
{
    if (OnLobbyReadyPhaseChanged.IsBound())
    {
        OnLobbyReadyPhaseChanged.Broadcast(LobbyReadyPhase);
    }
}

void AMSLobbyGameState::OnRep_RemainingTime()
{
    if (OnReadyTimeChanged.IsBound())
    {
        OnReadyTimeChanged.Broadcast(RemainingReadyTime);
    }
}
