// Fill out your copyright notice in the Description page of Project Settings.


#include "GameStates/MSLobbyGameState.h"
#include "Net/UnrealNetwork.h"

void AMSLobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMSLobbyGameState, RemainingReadyTime);

}

void AMSLobbyGameState::StartReadyCountdown(int32 StartSeconds)
{
    RemainingReadyTime = StartSeconds;

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
    if (RemainingReadyTime < 0)
    {
        GetWorld()->GetTimerManager().ClearTimer(ReadyTimerHandle);
        OnReadyCountdownFinished();
        return;
    }

    --RemainingReadyTime;
}

void AMSLobbyGameState::OnReadyCountdownFinished()
{
    //인게임 레벨로 이동
    FString MapURL = TEXT("GameLevel?listen");
    GetWorld()->ServerTravel(MapURL, true);
}

void AMSLobbyGameState::OnRep_RemainingTime()
{
    if (OnReadyTimeChanged.IsBound())
    {
        OnReadyTimeChanged.Broadcast(RemainingReadyTime);
    }
}
