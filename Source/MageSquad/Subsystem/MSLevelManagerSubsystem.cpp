// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/MSLevelManagerSubsystem.h"
#include <Kismet/GameplayStatics.h>


void UMSLevelManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    LobyLevelURL = FString::Printf(TEXT("/Game/Level/LobyLevel?listen"));
    LoadingLevelURL = FString::Printf(TEXT("/Game/Level/LoadingLevel"));
}

void UMSLevelManagerSubsystem::Deinitialize()
{

    Super::Deinitialize();
}

void UMSLevelManagerSubsystem::TravelToLoadingLevel()
{
    UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("LoadingLevel")));
}

void UMSLevelManagerSubsystem::HostGameAndTravelToLobby()
{
    GetWorld()->ServerTravel(LobyLevelURL, true);
}
