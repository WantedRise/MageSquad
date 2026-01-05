// Copyright (c) 2025 MageSquad

#include "Widgets/HUD/MSSkillSlotWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

#include "Materials/MaterialInstanceDynamic.h"

void UMSSkillSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 시작 시에는 빈 슬롯 상태로 설정
	SetEmpty();
}

void UMSSkillSlotWidget::SetEmpty()
{
	if (IconImageWidget)
	{
		// 아이콘 숨기기
		IconImageWidget->SetVisibility(ESlateVisibility::Hidden);

		// 머티리얼 인스턴스 초기화 해제
		IconMID = nullptr;
	}

	if (SkillLevelTextWidget)
	{
		// 스킬 레벨 텍스트 숨기기
		SkillLevelTextWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UMSSkillSlotWidget::BindSkill(const FMSHUDSkillSlotData& Data)
{
	if (!IconImageWidget || !SkillLevelTextWidget) return;

	// 스킬 데이터가 유효하지 않으면 빈 슬롯으로 전환
	if (!Data.bIsValid)
	{
		SetEmpty();
		return;
	}

	UMaterialInterface* Mat = nullptr;
	if (!Data.Icon.IsNull())
	{
		// 아이콘 머티리얼 동기 로딩
		Mat = Data.Icon.LoadSynchronous();
	}

	if (Mat)
	{
		// 머티리얼 인스턴스 동적 생성
		IconMID = UMaterialInstanceDynamic::Create(Mat, this);
		if (IconMID)
		{
			// 초기 스킬 쿨다운 퍼센트 파라미터 설정. 사용 가능 상태(0)
			IconMID->SetScalarParameterValue(FName(TEXT("CoolDownRemainingPercent")), 0.f);

			// 머티리얼을 브러시로 설정
			FSlateBrush Brush;
			Brush.SetResourceObject(IconMID);
			IconImageWidget->SetBrush(Brush);
			IconImageWidget->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			// 머티리얼 인스턴스 생성 실패 시 아이콘 숨김
			IconImageWidget->SetVisibility(ESlateVisibility::Hidden);
			return;
		}
	}
	else
	{
		// 머티리얼이 없으면 아이콘 숨김
		IconMID = nullptr;
		IconImageWidget->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	// 레벨 텍스트 설정 및 표시
	SkillLevelTextWidget->SetText(FText::AsNumber(Data.Level));
	SkillLevelTextWidget->SetVisibility(ESlateVisibility::Visible);
}

void UMSSkillSlotWidget::SetCooldownPercent(float InPercent)
{
	if (IconMID)
	{
		// 스킬 쿨다운 퍼센트 파라미터 설정
		const float Clamped = FMath::Clamp(InPercent, 0.f, 1.f);
		IconMID->SetScalarParameterValue(FName(TEXT("CoolDownRemainingPercent")), Clamped);
	}
}
