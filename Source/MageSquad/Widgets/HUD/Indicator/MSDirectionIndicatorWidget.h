// Copyright (c) 2025 MageSquad

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MSDirectionIndicatorWidget.generated.h"

// 인디케이터 대상과 상태 맵 구조체
USTRUCT(BlueprintType)
struct FIndicatorItem
{
	GENERATED_BODY()

	// 위젯 숨김 여부
	bool bVisible = false;

	// 스크린에 표시할 상대 위치
	FVector2D DesiredPos = FVector2D::ZeroVector;

	// 거리
	float Distance = 0.f;

	// 인디케이터 아이템과 매칭되는 인디케이터 컴포넌트
	TWeakObjectPtr<class UMSDirectionIndicatorComponent> IndicatorComponent;

	// 인디케이터 아이템 위젯
	TObjectPtr<class UMSDirectionIndicatorItemWidget> IndicatorItem = nullptr;
};

/*
 * 작성자: 김준형
 * 작성일: 26/01/02
 *
 * 방향 표시 인디케이터 위젯
 * 화면 밖으로 나간 대상(플레이어/임무/보스 등)에 대해 방향과 아이콘을 화면 가장자리에서 표시하는 기능을 제공
 * 방향 표시 서브시스템을 통해 표시할 목록을 받아와서 일정 주기로 계산하여 UI 갱신
 */
UCLASS()
class MAGESQUAD_API UMSDirectionIndicatorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	// 풀에서 인디케이터 아이템 위젯을 가져오거나 초기화하는 함수
	class UMSDirectionIndicatorItemWidget* AcquireItemWidget();

	// 인디케이터 아이템 위젯을 풀에 반환하는 함수
	void ReleaseItemWidget(class UMSDirectionIndicatorItemWidget* Widget);

	// 방향 표시 인디케이터 갱신 함수
	void UpdateIndicator();

protected:
	// 루트 캔버스 위젯
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Transient)
	TObjectPtr<class UCanvasPanel> RootCanvasWidget;

	// 인디케이터 아이템 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Indicator")
	TSubclassOf<class UMSDirectionIndicatorItemWidget> IndicatorItemWidgetClass;

	// 인디케이터 표시 시작 임계값(px). 대상의 스크린 좌표가 이 범위 밖으로 나가면 표시 시작
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Indicator")
	float ActivationInsetPixel = 150.f;

	// 인디케이터 표시 해제 임계값(px). 대상의 스크린 좌표가 이 범위 안으로 들어오면 표시 해제
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Indicator")
	float DeactivationInsetPixel = 300.f;

	// 화면 가장자리 마진값(px). 인디케이터가 화면 끝에서 이만큼 떨어진 곳에 위치 시킴
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Indicator")
	float EdgeMarginPixel = 80.f;

	// 인디케이터 간 최소 간격(px)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Indicator")
	float SlotGapPixel = 8.f;

	// 방향 표시 인디케이터 갱신 주기
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Indicator")
	float UpdateInterval = 0.05f;

	// 거리 표시 여부
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Indicator")
	bool bShowDistance = false;

private:
	// 인디케이터 갱신 타이머
	FTimerHandle UpdateTimerHandle;

	// 사용 중이지 않은 IndicatorItem 위젯 풀
	UPROPERTY(Transient)
	TArray<TObjectPtr<class UMSDirectionIndicatorItemWidget>> IndicatorItemPool;

	/*
	* 활성화된 인디케이터 목록
	* Key = UMSDirectionIndicatorComponent
	* Value = FIndicatorItem
	*/
	UPROPERTY(Transient)
	TMap<TWeakObjectPtr<class UMSDirectionIndicatorComponent>, FIndicatorItem> ActiveIndicators;
};