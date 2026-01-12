// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/HUD/MSOverheadNameWidget.h"

#include "Components/TextBlock.h"

void UMSOverheadNameWidget::SetNameText(const FText& InText)
{
	if (NameText)
	{
		NameText->SetText(InText);
	}
}
