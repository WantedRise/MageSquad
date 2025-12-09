// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Lobby/MSLobbyPlayerSlotWidget.h"
#include "Components/Button.h"
#include "Subsystem/MSSteamManagerSubsystem.h"

void UMSLobbyPlayerSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (Button_Invite)
	{
		Button_Invite->OnClicked.AddDynamic(this, &UMSLobbyPlayerSlotWidget::ShowFriendList);
	}
}
void UMSLobbyPlayerSlotWidget::ShowFriendList()
{
	UE_LOG(LogTemp, Warning, TEXT("ShowFriendList successfully!"));
	UMSSteamManagerSubsystem* MSSteamManagerSubsystem = GetGameInstance()->GetSubsystem<UMSSteamManagerSubsystem>();
	if (MSSteamManagerSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("MSSteamManagerSubsystem successfully!"));
		MSSteamManagerSubsystem->ShowFriendInvitationScreen();
	}
}
