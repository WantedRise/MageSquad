// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/MSLobbyGameMode.h"
#include "EngineUtils.h"
#include "Actors/MSLobbyPlayerSlot.h"
#include "System/MSSteamManagerSubsystem.h"
#include "GameFramework/GameState.h"

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

void AMSLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{

    Super::PostLogin(NewPlayer);
}