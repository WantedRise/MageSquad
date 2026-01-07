// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Lobby/MSCharacterInfoWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Materials/MaterialInstance.h"


void UMSCharacterInfoWidget::UpdateInfoWidget(FText Title, FText Name, FText Decs, UTexture2D* Icon)
{
	if (Text_Title)
	{
		Text_Title->SetText(Title);
	}
	if (Text_Name)
	{
		Text_Name->SetText(Name);
	}
	if (Text_Decs)
	{
		Text_Decs->SetText(Decs);
	}
	if (Image_Icon)
	{
		Image_Icon->SetBrushFromTexture(Icon);
	}
}
void UMSCharacterInfoWidget::UpdateInfoWidget(FText Title, FText Name, FText Decs, TSoftObjectPtr <UMaterialInterface> Icon)
{
	if (Text_Title)
	{
		Text_Title->SetText(Title);
	}
	if (Text_Name)
	{
		Text_Name->SetText(Name);
	}
	if (Text_Decs)
	{
		Text_Decs->SetText(Decs);
	}
	if (Image_Icon && Icon.IsValid())
	{
		Image_Icon->SetBrushFromMaterial(Icon.Get());
	}
}