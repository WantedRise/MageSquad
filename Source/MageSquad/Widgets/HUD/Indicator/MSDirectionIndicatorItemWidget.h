// Copyright (c) 2025 MageSquad

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MSDirectionIndicatorItemWidget.generated.h"

/*
 * 작성자: 김준형
 * 작성일: 26/01/02
 *
 * 방향 표시 인디케이터의 한 항목을 표현하는 위젯
 * 방향 화살표, 아이콘, 사망 오버레이, 거리 텍스트 등을 구성
 */
UCLASS()
class MAGESQUAD_API UMSDirectionIndicatorItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

	// 이 위젯에 대한 인디케이터 컴포넌트를 설정하는 함수
	void SetIndicatorComponent(class UMSDirectionIndicatorComponent* InIndicator);

	// 플레이어: 사망 오버레이 표시/숨김 처리하는 함수
	void SetDeadOverlayVisible(bool bVisible);

	// 아이콘 설정 함수
	void SetIcon(UTexture2D* Texture);

	// 거리 표시 설정 함수
	void SetDistance(bool bVisible, float Meters);

	// 인디케이터 컴포넌트 Getter
	class UMSDirectionIndicatorComponent* GetIndicatorComponent() const { return IndicatorComponent; }

protected:
	/* ======================== BindWidget ======================== */
	// 아이콘 이미지 위젯
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Transient)
	TObjectPtr<class UImage> IconImageWidget;

	// 플레이어: 사망 오버레이 이미지 위젯
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Transient)
	TObjectPtr<class UImage> DeadOverlayImageWidget;

	// 거리 텍스트 위젯
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Transient)
	TObjectPtr<class UTextBlock> DistanceTextWidget;
	/* ======================== BindWidget ======================== */

	// 인디케이터 컴포넌트
	UPROPERTY(Transient)
	TObjectPtr<class UMSDirectionIndicatorComponent> IndicatorComponent;

private:
	// 거리 텍스트 표시 여부
	bool bDistanceVisible = false;
	float CurrentDistance = 0.f;
};