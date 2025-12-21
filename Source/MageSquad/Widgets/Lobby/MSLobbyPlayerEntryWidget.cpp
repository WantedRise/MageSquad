// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Lobby/MSLobbyPlayerEntryWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UMSLobbyPlayerEntryWidget::ShowHostIcon()
{
	if (Image_HostIcon)
	{
		Image_HostIcon->SetVisibility(ESlateVisibility::Visible);
	}
}

void UMSLobbyPlayerEntryWidget::SetTextNickName(FString InNickName)
{
	if (Text_NickName)
	{
		Text_NickName->SetText(FText::FromString(InNickName));
	}
}

void UMSLobbyPlayerEntryWidget::SetTextReadyStatus(bool bReady)
{
	if (Text_ReadyStatus)
	{
		if (bReady)
		{
			Text_ReadyStatus->SetText(FText::FromString(TEXT("준비")));
			Text_ReadyStatus->SetColorAndOpacity(FLinearColor::Yellow);
		}
		else
		{
			Text_ReadyStatus->SetText(FText::FromString(TEXT("준비되지 않음")));
			Text_ReadyStatus->SetColorAndOpacity(FLinearColor::Red);
		}
	}
}