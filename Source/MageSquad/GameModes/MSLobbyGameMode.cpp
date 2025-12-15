// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/MSLobbyGameMode.h"
#include "EngineUtils.h"
#include "Actors/MSLobbyPlayerSlot.h"
#include "System/MSSteamManagerSubsystem.h"
#include "GameFramework/GameState.h"
#include <Player/MSLobbyPlayerState.h>
#include "MageSquad.h"
#include "GameStates/MSLobbyGameState.h"

AMSLobbyGameMode::AMSLobbyGameMode()
{

}

AActor* AMSLobbyGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
    UWorld* World = GetWorld();
    if (World)
    {
        for (AMSLobbyPlayerSlot* PlayerSlot : TActorRange<AMSLobbyPlayerSlot>(World))
        {
            if (IsValid(PlayerSlot) && nullptr == PlayerSlot->GetController())
            {
                UE_LOG(LogTemp, Warning, TEXT("ChoosePlayerStart : %s"), *PlayerSlot->GetName());
                PlayerSlot->SetController(Player);
                PlayerSlot->HiddenInviteWidgetComponent();
                
                return PlayerSlot;
            }
        }
    }
	return nullptr;
}

void AMSLobbyGameMode::HandlePlayerReadyStateChanged()
{
    int32 ReadyCount = 0;
    int32 TotalPlayers = 0;

    //모든 PlayerState를 조회하여 레디상태인지 확인
    for (APlayerState* PS : GameState->PlayerArray)
    {
        if (AMSLobbyPlayerState* LobbyPS = Cast<AMSLobbyPlayerState>(PS))
        {
            ++TotalPlayers;
            if (LobbyPS->IsReady())
            {
                ++ReadyCount;
            }
        }
    }

    if (ReadyCount == 0)
    {
        return;
    }
    
    AMSLobbyGameState* LobbyGS = Cast<AMSLobbyGameState>(GameState);

    if (nullptr == LobbyGS)
    {
        MS_LOG(LogMSNetwork, Warning, TEXT("%s"), TEXT("AMSLobbyGameState nullptr"));
        return;
    }

    int32 StartCount = 0;
    if (TotalPlayers == ReadyCount)
    {
        StartCount = 3;
    }
    else
    {
        StartCount = 60;
    }

    LobbyGS->StartReadyCountdown(60);
}


