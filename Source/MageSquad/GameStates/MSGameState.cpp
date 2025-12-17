// Fill out your copyright notice in the Description page of Project Settings.


#include "GameStates/MSGameState.h"
#include "GameModes/MSGameMode.h"
#include "GameStates/GameFlow/MSGameFlowBase.h"
#include "MageSquad.h"
void AMSGameState::BeginPlay()
{
	Super::BeginPlay();

    if (HasAuthority())
    {
        if (AMSGameMode* GM = GetWorld()->GetAuthGameMode<AMSGameMode>())
        {
            TSubclassOf<class UMSGameFlowBase> GameFlowToSet = GM->GetGameFlowClass();
            if (GameFlowToSet)
            {
                GameFlow = NewObject<UMSGameFlowBase>(this, GameFlowToSet);
                GameFlow->Initialize(this);
            }
        }
        else
        {
            UE_LOG(LogMSNetwork, Warning, TEXT("Server: GameFlowClass is not set in GameMode"));
        }
    }
}
