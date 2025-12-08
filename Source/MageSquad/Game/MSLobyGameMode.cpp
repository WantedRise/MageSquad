// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/MSLobyGameMode.h"
#include "EngineUtils.h"
#include "Actors/MSLobyPlayerSlot.h"

AActor* AMSLobyGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
    //UWorld* World = GetWorld();
    //if (World)
    //{
    //    for (AMSLobyPlayerSlot* PlayerSlot : TActorRange<AMSLobyPlayerSlot>(World))
    //    {
    //        if (IsValid(PlayerSlot) && nullptr==PlayerSlot->GetController())
    //        {
    //            UE_LOG(LogTemp, Warning, TEXT("찾은 캐릭터 이름: %s"), *PlayerSlot->GetName());
    //            PlayerSlot->SetController(Player);
    //            return PlayerSlot;
    //        }
    //    }
    //}

	return nullptr;
}

void AMSLobyGameMode::PostLogin(APlayerController* NewPlayer)
{
    UWorld* World = GetWorld();
    if (World)
    {
        for (AMSLobyPlayerSlot* PlayerSlot : TActorRange<AMSLobyPlayerSlot>(World))
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