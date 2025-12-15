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
    bUseSeamlessTravel = true;
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

void AMSLobbyGameMode::HandleReadyCountdownFinished()
{
    UE_LOG(LogTemp, Warning, TEXT("Lobby -> GameLevel ServerTravel"));

    GetWorld()->ServerTravel(
        TEXT("GameLevel?listen")
    );
}

void AMSLobbyGameMode::HandlePlayerReadyStateChanged()
{
    AMSLobbyGameState* LobbyGS = Cast<AMSLobbyGameState>(GameState);
    if (nullptr == LobbyGS)
    {
        MS_LOG(LogMSNetwork, Warning, TEXT("%s"), TEXT("AMSLobbyGameState nullptr"));
        return;
    }

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

    ELobbyReadyPhase NewPhase =
        (ReadyCount == 0) ? ELobbyReadyPhase::NotReady :
        (ReadyCount == TotalPlayers) ? ELobbyReadyPhase::AllReady :
        ELobbyReadyPhase::PartialReady;

    // Phase 변경 시에만 처리
    if (LobbyGS->GetLobbyReadyPhase() != NewPhase)
    {
        LobbyGS->SetLobbyReadyPhase(NewPhase);
        LobbyGS->OnRep_LobbyReadyPhase(); // 리슨서버 즉시 반영용
        MS_LOG(LogMSNetwork, Warning, TEXT("%d"), (int32)NewPhase);
        switch (NewPhase)
        {
        case ELobbyReadyPhase::NotReady:
            LobbyGS->StopReadyCountdown();
            break;

        case ELobbyReadyPhase::PartialReady:
            LobbyGS->StartReadyCountdown(60);
            break;

        case ELobbyReadyPhase::AllReady:
            LobbyGS->StartReadyCountdown(3);
            break;
        }
    }
}


