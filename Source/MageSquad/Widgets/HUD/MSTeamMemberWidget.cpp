// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/HUD/MSTeamMemberWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/HorizontalBox.h"

#include "Widgets/HUD/MSSkillSlotWidget.h"

void UMSTeamMemberWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 팀원 스킬 슬롯을 1회 생성/부착 후, 이후에는 데이터만 갱신
	EnsureSkillSlotsInitialized();
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
	if (!HUDData) return;

	// 팀원 스킬 슬롯 위젯 한번 더 보정
	EnsureSkillSlotsInitialized();
	if (!bSkillSlotsInitialized) return;

	// 팀원의 복제된 스킬 슬롯 데이터를 가져옴
	const TArray<FMSHUDSkillSlotData>& ViewData = HUDData->GetSkillSlotData();

	// 슬롯 인덱스에 해당하는 위젯에 데이터를 반영
	for (int32 SlotIndex = 0; SlotIndex < 6; ++SlotIndex)
	{
		const FMSHUDSkillSlotData* SlotData = ViewData.IsValidIndex(SlotIndex) ? &ViewData[SlotIndex] : nullptr;
		UpdateSkillSlotWidget(SlotIndex, SlotData);
	}
}

float UMSTeamMemberWidget::CalcHealthPct(float InHealth, float InMaxHealth)
{
	return (InMaxHealth > 0.f) ? FMath::Clamp(InHealth / InMaxHealth, 0.f, 1.f) : 0.f;
}

void UMSTeamMemberWidget::EnsureSkillSlotsInitialized()
{
	if (bSkillSlotsInitialized) return;

	// 팀원 스킬 슬롯 위젯 클래스 가져오기
	TSubclassOf<UMSSkillSlotWidget> WidgetClass = SkillSlotWidgetClass;
	if (!WidgetClass)
	{
		WidgetClass = UMSSkillSlotWidget::StaticClass();
	}

	// 팀원 스킬 슬롯 동적 생성
	// - 슬롯 위젯은 먼저 패널에 AddChild되어 Construct가 완료된 뒤에 BindSkill을 호출해야 함
	TeamSkillWidgets.Reset(6);
	ActiveSkillListWidget->ClearChildren();
	PassiveSkillListWidget->ClearChildren();

	for (int32 SlotIndex = 0; SlotIndex < 6; ++SlotIndex)
	{
		// 스킬 슬롯 생성
		UMSSkillSlotWidget* NewSlot = CreateWidget<UMSSkillSlotWidget>(GetOwningPlayer(), WidgetClass);
		if (!NewSlot)
		{
			// 부분 생성된 경우가 있다면 정리
			ActiveSkillListWidget->ClearChildren();
			PassiveSkillListWidget->ClearChildren();
			TeamSkillWidgets.Reset();
			return;
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

		NewSlot->SetEmpty();
		TeamSkillWidgets.Add(NewSlot);
	}

	bSkillSlotsInitialized = true;
}

void UMSTeamMemberWidget::UpdateSkillSlotWidget(const int32 SlotIndex, const FMSHUDSkillSlotData* Data)
{
	if (!TeamSkillWidgets.IsValidIndex(SlotIndex)) return;

	// 팀원 스킬 슬롯 위젯 목록에서 해당 슬롯 인덱스의 위젯 가져오기
	UMSSkillSlotWidget* SlotWidget = TeamSkillWidgets[SlotIndex];
	if (!SlotWidget) return;

	// 유효한 스킬 데이터면 바인딩, 아니면 Empty
	if (Data && Data->bIsValid)
	{
		SlotWidget->BindSkill(*Data);

		// 팀원 쿨다운은 별도 복제가 없으므로 0으로 초기화
		SlotWidget->SetCooldownPercent(0.f);
	}
	else
	{
		SlotWidget->SetEmpty();
	}
}
