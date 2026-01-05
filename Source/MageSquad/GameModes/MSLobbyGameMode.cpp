// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/MSLobbyGameMode.h"
#include "EngineUtils.h"
#include "Actors/MSLobbyPlayerSlot.h"
#include "System/MSSteamManagerSubsystem.h"
#include "GameFramework/GameState.h"
#include <Player/MSLobbyPlayerState.h>
#include "MageSquad.h"
#include "GameStates/MSLobbyGameState.h"
#include <System/MSLevelManagerSubsystem.h>
#include "Player/MSLobbyPlayerController.h"
#include <System/MSCharacterDataSubsystem.h>

AMSLobbyGameMode::AMSLobbyGameMode()
{
    bUseSeamlessTravel = true;
}
void AMSLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    MS_LOG(LogMSNetwork, Log,TEXT("%s"), TEXT("TEST"));
    if (AMSLobbyPlayerState* PS = NewPlayer->GetPlayerState<AMSLobbyPlayerState>())
    {
        if (GameState->PlayerArray.Num() == 1)
        {
            //호스트 표시
            PS->SetHost(true);
        }

        // 이미 값이 있으면 건드리지 않음
        if (PS->GetSelectedCharacterID() != NAME_None)
            return;

        UMSCharacterDataSubsystem* CharacterData = GetGameInstance()->GetSubsystem<UMSCharacterDataSubsystem>();
        if (!CharacterData) return;

        const FName DefaultCharacterID = CharacterData->GetDefaultCharacterID();

        if (DefaultCharacterID == NAME_None)
            return;

        // ⭐ 디폴트 캐릭터 설정
        PS->SetSelectedCharacter(DefaultCharacterID);
    }
}

AActor* AMSLobbyGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
    UWorld* World = GetWorld();
    if (World)
    {
        //플레이어 슬롯을 스폰 위치로 지정
        for (AMSLobbyPlayerSlot* PlayerSlot : TActorRange<AMSLobbyPlayerSlot>(World))
        {
            if (IsValid(PlayerSlot) && nullptr == PlayerSlot->GetController())
            {
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
    //클라이언트들에게 로딩창 띄우라고 명령
    if (AMSLobbyGameState* LobbyGS = Cast<AMSLobbyGameState>(GameState))
    {
        LobbyGS->Multicast_ShowLoadingScreen();
    }

    //호스트 로딩창 띄우기
    if (UMSLevelManagerSubsystem* LevelManager = GetGameInstance()->GetSubsystem<UMSLevelManagerSubsystem>())
    {
        LevelManager->ShowLoadingWidget();
        GetWorld()->ServerTravel(LevelManager->GetGameLevelURL());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("UMSLevelManagerSubsystem is nullptr"));
    }
    
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

    //모든 PlayerState를 조회하여 준비상태인지 확인
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


