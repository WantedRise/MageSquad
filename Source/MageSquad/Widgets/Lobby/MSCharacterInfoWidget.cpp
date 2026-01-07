// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Lobby/MSCharacterInfoWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/HorizontalBox.h"


void UMSCharacterInfoWidget::UpdateInfoWidget(FText Title, FText Name, FText Decs, UTexture2D* Icon)
{
	if (Title.IsEmpty())
	{
		HorizontalBox_Title->SetVisibility(ESlateVisibility::Collapsed);
		Image_divider->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		Text_Title->SetText(Title);
	}
	if (Text_Name)
	{
		Text_Name->SetText(Name);
		Text_Name->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 0.08f, 1.0f));
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

	check(Image_Icon);

	UMaterialInterface* Mat = nullptr;
	if (!Icon.IsNull())
	{
		// 아이콘 머티리얼 동기 로딩
		Mat = Icon.LoadSynchronous();
	}

	if (Mat)
	{
		// 머티리얼 인스턴스 동적 생성
		SkillIconMID = UMaterialInstanceDynamic::Create(Mat, this);
		if (SkillIconMID)
		{
			// 초기 스킬 쿨다운 퍼센트 파라미터 설정. 사용 가능 상태(0)
			SkillIconMID->SetScalarParameterValue(FName(TEXT("CoolDownRemainingPercent")), 0.f);

			// 기존 브러시를 유지한 채 리소스만 교체
			FSlateBrush Brush = Image_Icon->GetBrush();
			Brush.SetResourceObject(SkillIconMID);
			Image_Icon->SetBrush(Brush);
			Image_Icon->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			// 머티리얼 인스턴스 생성 실패 시 아이콘 숨김
			Image_Icon->SetVisibility(ESlateVisibility::Hidden);
			return;
		}
	}
	else
	{
		// 머티리얼이 없으면 아이콘 숨김
		SkillIconMID = nullptr;
		Image_Icon->SetVisibility(ESlateVisibility::Hidden);
		return;
	}
}