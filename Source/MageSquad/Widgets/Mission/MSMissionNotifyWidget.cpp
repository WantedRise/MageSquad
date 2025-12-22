// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Mission/MSMissionNotifyWidget.h"
#include "Components/TextBlock.h"

void UMSMissionNotifyWidget::SetMissionMessage(FText InMessage)
{
	Text_MissionMessage->SetText(InMessage);
}
