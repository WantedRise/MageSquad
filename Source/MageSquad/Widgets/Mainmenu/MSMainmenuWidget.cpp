// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Mainmenu/MSMainmenuWidget.h"
#include "Components/Button.h"
#include "Subsystem/MSLevelManagerSubsystem.h"
#include "Subsystem/MSSteamManagerSubsystem.h"

UMSMainmenuWidget::UMSMainmenuWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
 
}

void UMSMainmenuWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (Button_Play)
    {
        Button_Play->OnClicked.AddDynamic(this, &UMSMainmenuWidget::CreateSessionAndTravalToLobby);
    }
}

void UMSMainmenuWidget::CreateSessionAndTravalToLobby()
{
    UMSSteamManagerSubsystem* SteamManager = GetGameInstance()->GetSubsystem<UMSSteamManagerSubsystem>();
    if (SteamManager)
    {
        SteamManager->CreateSteamSession(true, 4);
    }

    UMSLevelManagerSubsystem* LevelManagerSubsystem = GetGameInstance()->GetSubsystem<UMSLevelManagerSubsystem>();
    if (LevelManagerSubsystem)
    {
        LevelManagerSubsystem->HostGameAndTravelToLobby();
    }
}
