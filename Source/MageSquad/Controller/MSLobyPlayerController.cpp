// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/MSLobyPlayerController.h"
#include "Subsystem/MSSteamManagerSubsystem.h"

AMSLobyPlayerController::AMSLobyPlayerController()
{
}


void AMSLobyPlayerController::BeginPlay()
{
	Super::BeginPlay();

    UMSSteamManagerSubsystem* SteamManager = GetGameInstance()->GetSubsystem<UMSSteamManagerSubsystem>();
    if (SteamManager)
    {
        SteamManager->CreateSteamSession(true, 4);
    }
}
