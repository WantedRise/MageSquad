// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MSTeamMemberWidget.generated.h"

/**
 * 작성자: 김준형
 * 작성일: 25/12/18
 *
 * 플레이어 HUD에 표시되는 팀 멤버들의 데이터 위젯
 * - 팀원 체력바
 * - 팀원 아이콘
 * - 팀원 이름
 * - 팀원 스킬 목록
 */
UCLASS()
class MAGESQUAD_API UMSTeamMemberWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// HUD 데이터 갱신 함수 (HUDDataComponent로부터 값을 읽어 UI를 갱신)
	void UpdateFromHUDData(const class UMSHUDDataComponent* HUDData);

	// 직접 값으로 갱신하는 함수
	void UpdateHealth(float InHealth, float InMaxHealth);
	void UpdateName(const FText& InName);
	void UpdatePortrait(UTexture2D* InPortrait);

private:
	// 체력 비율 계산 함수
	static float CalcHealthPct(float InHealth, float InMaxHealth);

protected:
	/* ======================== BindWidget ======================== */
	// 팀원 체력 바 위젯
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Transient)
	TObjectPtr<class UProgressBar> TeamHealthBarWidget;

	// 팀원 이름 텍스트 위젯
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Transient)
	TObjectPtr<class UTextBlock> TeamNameTextWidget;

	// 팀원 아이콘 위젯
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Transient)
	TObjectPtr<class UImage> TeamPortraitIconWidget;
	/* ======================== BindWidget ======================== */
};
