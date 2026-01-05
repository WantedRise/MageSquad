// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/HUD/MSTeamMemberWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/HorizontalBox.h"
#include "Components/Player/MSHUDDataComponent.h"

#include "Widgets/HUD/MSSkillSlotWidget.h"

UMSTeamMemberWidget::UMSTeamMemberWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	// 팀 멤버 스킬 슬롯의 기본 위젯 클래스를 설정
	SkillSlotWidgetClass = UMSSkillSlotWidget::StaticClass();
}

void UMSTeamMemberWidget::UpdateFromHUDData(const UMSHUDDataComponent* HUDData)
{
	if (!HUDData) return;

	UpdateHealth(HUDData->GetHealth(), HUDData->GetMaxHealth());
	UpdateName(HUDData->GetDisplayName());
	UpdatePortrait(HUDData->GetPortraitIcon());
	UpdateSkills(HUDData);
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

void UMSTeamMemberWidget::UpdateSkills(const UMSHUDDataComponent* HUDData)
{
	// 팀원의 복제된 스킬 슬롯 데이터를 가져옴
	const TArray<FMSHUDSkillSlotData>& ViewData = HUDData->GetSkillSlotData();

	// 기존 스킬 목록 초기화
	SkillListWidget->ClearChildren();
	TeamSkillWidgets.Empty();

	TSubclassOf<UMSSkillSlotWidget> WidgetClass = SkillSlotWidgetClass;
	if (!WidgetClass)
	{
		WidgetClass = UMSSkillSlotWidget::StaticClass();
	}

	for (const FMSHUDSkillSlotData& Data : ViewData)
	{
		// 새 스킬 슬롯 생성
		UMSSkillSlotWidget* NewSlot = CreateWidget<UMSSkillSlotWidget>(GetWorld(), WidgetClass);
		if (!NewSlot) continue;

		// 슬롯이 유효한 경우에만 데이터를 채움
		if (Data.bIsValid)
		{
			NewSlot->BindSkill(Data);
			NewSlot->SetCooldownPercent(0.f);
		}

		SkillListWidget->AddChild(NewSlot);
		TeamSkillWidgets.Add(NewSlot);
	}
}

float UMSTeamMemberWidget::CalcHealthPct(float InHealth, float InMaxHealth)
{
	return (InMaxHealth > 0.f) ? FMath::Clamp(InHealth / InMaxHealth, 0.f, 1.f) : 0.f;
}
