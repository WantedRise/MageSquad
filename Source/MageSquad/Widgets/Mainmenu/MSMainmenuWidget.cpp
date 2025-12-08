// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Mainmenu/MSMainmenuWidget.h"
#include "Components/Button.h"
#include "Subsystem/MSLevelManagerSubsystem.h"


UMSMainmenuWidget::UMSMainmenuWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
 
}

void UMSMainmenuWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (Button_Play)
    {
        Button_Play->OnClicked.AddDynamic(this, &UMSMainmenuWidget::CreateSessionAndTravalToLoby);
    }
}

void UMSMainmenuWidget::CreateSessionAndTravalToLoby()
{


    UMSLevelManagerSubsystem* LevelManagerSubsystem = GetGameInstance()->GetSubsystem<UMSLevelManagerSubsystem>();
    if (LevelManagerSubsystem)
    {
        LevelManagerSubsystem->HostGameAndTravelToLobby();
    }
}
