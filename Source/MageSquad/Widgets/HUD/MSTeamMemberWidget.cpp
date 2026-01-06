// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/HUD/MSTeamMemberWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/HorizontalBox.h"

#include "Widgets/HUD/MSSkillSlotWidget.h"

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
	if (TeamPortraitIconWidget)
	{
		if (InPortrait)
		{
			TeamPortraitIconWidget->SetBrushFromTexture(InPortrait);
		}
		else
		{
			// 초상화가 없으면 기본값이나 비워둔다.
			TeamPortraitIconWidget->SetBrushFromTexture(nullptr);
		}
	}
}

void UMSTeamMemberWidget::UpdateSkills(const UMSHUDDataComponent* HUDData)
{
	// 팀원의 복제된 스킬 슬롯 데이터를 가져옴
	const TArray<FMSHUDSkillSlotData>& ViewData = HUDData->GetSkillSlotData();

	// 기존 스킬 목록 초기화
	PassiveSkillListWidget->ClearChildren();
	ActiveSkillListWidget->ClearChildren();
	TeamSkillWidgets.Empty();

	// 0~1번 슬롯은 액티브, 2~5번 슬롯은 패시브로 간주
	const int32 NumView = ViewData.Num();

	// 액티브 스킬: Index 0~1
	for (int32 i = 0; i < 2 && i < NumView; ++i)
	{
		const FMSHUDSkillSlotData& Data = ViewData[i];
		if (Data.bIsValid)
		{
			// 스킬 위젯 추가
			AddSkillWidget(Data, i);
		}
	}

	// 패시브 스킬: Index 2~5
	for (int32 i = 2; i < 6 && i < NumView; ++i)
	{
		const FMSHUDSkillSlotData& Data = ViewData[i];
		if (Data.bIsValid)
		{
			// 스킬 위젯 추가
			AddSkillWidget(Data, i);
		}
	}
}

float UMSTeamMemberWidget::CalcHealthPct(float InHealth, float InMaxHealth)
{
	return (InMaxHealth > 0.f) ? FMath::Clamp(InHealth / InMaxHealth, 0.f, 1.f) : 0.f;
}

void UMSTeamMemberWidget::AddSkillWidget(const FMSHUDSkillSlotData& Data, const int32 SlotIndex)
{
	TSubclassOf<UMSSkillSlotWidget> WidgetClass = SkillSlotWidgetClass;
	if (!WidgetClass)
	{
		WidgetClass = UMSSkillSlotWidget::StaticClass();
	}

	// 새 스킬 슬롯 위젯 생성
	UMSSkillSlotWidget* NewSlot = CreateWidget<UMSSkillSlotWidget>(GetWorld(), WidgetClass);
	if (NewSlot) return;

	// 스킬 데이터가 유효하면 아이콘과 레벨을 바인딩하고 쿨다운을 초기화
	if (Data.bIsValid)
	{
		NewSlot->BindSkill(Data);
		NewSlot->SetCooldownPercent(0.f);
	}
	else
	{
		// 비어있는 슬롯은 빈 상태로 설정
		NewSlot->SetEmpty();
	}

	// 스킬 슬롯 위젯에 추가 (패시브, 액티브 구분)
	if (SlotIndex > 1)
	{
		PassiveSkillListWidget->AddChild(NewSlot);
	}
	else
	{
		ActiveSkillListWidget->AddChild(NewSlot);
	}

	// 캐시 목록에 보관
	TeamSkillWidgets.Add(NewSlot);
}
