// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/HUD/MSTeamMemberWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

#include "Components/Player/MSHUDDataComponent.h"

void UMSTeamMemberWidget::UpdateFromHUDData(const UMSHUDDataComponent* HUDData)
{
	if (!HUDData) return;

	UpdateHealth(HUDData->GetHealth(), HUDData->GetMaxHealth());
	UpdateName(HUDData->GetDisplayName());
	UpdatePortrait(HUDData->GetPortraitIcon());
}

void UMSTeamMemberWidget::UpdateHealth(float InHealth, float InMaxHealth)
{
	// 체력 비율 계산
	const float Pct = CalcHealthPct(InHealth, InMaxHealth);

	// 체력 바 갱신
	if (TeamHealthBarWidget)
	{
		TeamHealthBarWidget->SetPercent(Pct);
	}
}

void UMSTeamMemberWidget::UpdateName(const FText& InName)
{
	// 텍스트 갱신
	if (TeamNameTextWidget)
	{
		TeamNameTextWidget->SetText(InName);
	}
}

void UMSTeamMemberWidget::UpdatePortrait(UTexture2D* InPortrait)
{
	// 아이콘 갱신
	if (TeamPortraitIconWidget || InPortrait)
	{
		TeamPortraitIconWidget->SetBrushFromTexture(InPortrait);
	}
}

float UMSTeamMemberWidget::CalcHealthPct(float InHealth, float InMaxHealth)
{
	return (InMaxHealth > 0.f) ? FMath::Clamp(InHealth / InMaxHealth, 0.f, 1.f) : 0.f;
}
