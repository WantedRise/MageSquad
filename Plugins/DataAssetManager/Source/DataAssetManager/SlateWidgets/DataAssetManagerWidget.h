// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/SCompoundWidget.h"

class SDataAssetManagerTab : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SDataAssetManagerTab) {}

	SLATE_ARGUMENT(FString, TestString)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);
};
