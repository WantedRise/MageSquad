// Fill out your copyright notice in the Description page of Project Settings.


#include "System/MSLevelManagerSubsystem.h"
#include <Kismet/GameplayStatics.h>


void UMSLevelManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    LobbyLevelURL = TEXT("LobbyLevel?Listen");
    LoadingLevelURL = TEXT("LoadingLevel");
}

void UMSLevelManagerSubsystem::Deinitialize()
{

    Super::Deinitialize();
}

void UMSLevelManagerSubsystem::TravelToLoadingLevel()
{
    UGameplayStatics::OpenLevel(GetWorld(), FName(LoadingLevelURL));
}

void UMSLevelManagerSubsystem::HostGameAndTravelToLobby()
{
    GetWorld()->ServerTravel(LobbyLevelURL, false ,false);
}
