// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAssetManagerWidget.h"

void SDataAssetManagerTab::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;
	FSlateFontInfo TitleTextFont = FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));
	TitleTextFont.Size = 30;
	ChildSlot
	[
		// Main vertical box
		SNew(SVerticalBox)

		// First Vertical slot for Title Text
		+SVerticalBox::Slot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("DataAssetManager")))
			.Font(TitleTextFont)
			.Justification(ETextJustify::Center)
			.ColorAndOpacity(FColor::White)
		]
	];
}
