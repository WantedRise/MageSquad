// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/MSLobbyGameMode.h"
#include "EngineUtils.h"
#include "Actors/MSLobbyPlayerSlot.h"

AMSLobbyGameMode::AMSLobbyGameMode()
{

}

AActor* AMSLobbyGameMode::ChoosePlayerStart_Implementation(AController* Player)
{

	return nullptr;
}

void AMSLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
    UWorld* World = GetWorld();
    if (World)
    {
        for (AMSLobbyPlayerSlot* PlayerSlot : TActorRange<AMSLobbyPlayerSlot>(World))
        {
            if (IsValid(PlayerSlot) && nullptr == PlayerSlot->GetController())
            {
                UE_LOG(LogTemp, Warning, TEXT("찾은 캐릭터 이름: %s"), *PlayerSlot->GetName());
                PlayerSlot->SetController(NewPlayer);
                NewPlayer->StartSpot = PlayerSlot;
                break;
            }
        }
    }

    Super::PostLogin(NewPlayer);
}