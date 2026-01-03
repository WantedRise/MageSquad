// Copyright (c) 2025 MageSquad

#include "Widgets/HUD/Indicator/MSDirectionIndicatorItemWidget.h"

#include "Components/MSDirectionIndicatorComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UMSDirectionIndicatorItemWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	// 비가시성이 있는 위젯 프리뷰 초기화
	if (DistanceTextWidget)
	{
		DistanceTextWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (DeadOverlayImageWidget)
	{
		DeadOverlayImageWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UMSDirectionIndicatorItemWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 위젯 기본값 초기화
	if (IconImageWidget)
	{
		IconImageWidget->SetBrushFromTexture(nullptr);
	}
	if (DistanceTextWidget)
	{
		DistanceTextWidget->SetText(FText::GetEmpty());
		DistanceTextWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (DeadOverlayImageWidget)
	{
		DeadOverlayImageWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UMSDirectionIndicatorItemWidget::SetIndicatorComponent(UMSDirectionIndicatorComponent* InIndicator)
{
	if (InIndicator)
	{
		IndicatorComponent = InIndicator;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[UMSDirectionIndicatorItemWidget] Non - UMSDirectionIndicatorComponent"));
	}
}

void UMSDirectionIndicatorItemWidget::SetDeadOverlayVisible(bool bVisible)
{
	if (DeadOverlayImageWidget)
	{
		// 사망 오버레이 이미지 가시화 설정
		DeadOverlayImageWidget->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UMSDirectionIndicatorItemWidget::SetIcon(UTexture2D* Texture)
{
	if (IconImageWidget)
	{
		if (Texture)
		{
			IconImageWidget->SetBrushFromTexture(Texture);
			IconImageWidget->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			IconImageWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UMSDirectionIndicatorItemWidget::SetDistance(bool bVisible, float Meters)
{
	// 거리 텍스트 표시 여부 + 거리 초기화
	bDistanceVisible = bVisible;
	CurrentDistance = Meters;

	if (DistanceTextWidget)
	{
		if (bDistanceVisible)
		{
			// 길이를 int로 변환
			int32 Rounded = FMath::RoundToInt(Meters);
			DistanceTextWidget->SetText(FText::FromString(FString::Printf(TEXT("%dm"), Rounded)));
			DistanceTextWidget->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			DistanceTextWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}
