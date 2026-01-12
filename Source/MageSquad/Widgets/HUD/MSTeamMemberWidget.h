// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Player/MSHUDDataComponent.h"
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
	virtual void NativeConstruct() override;

	// HUD 데이터 갱신 함수 (HUDDataComponent로부터 값을 읽어 UI를 갱신)
	void UpdateFromHUDData(const class UMSHUDDataComponent* HUDData);

	// 직접 값으로 갱신하는 함수
	void UpdateHealth(float InHealth, float InMaxHealth);
	void UpdateName(const FText& InName);
	void UpdatePortrait(UTexture2D* InPortrait);
	void UpdateSkills(const class UMSHUDDataComponent* HUDData);

private:
	// 체력 비율 계산 함수
	static float CalcHealthPct(float InHealth, float InMaxHealth);

	// 팀원 스킬 슬롯 위젯 초기화 함수
	void EnsureSkillSlotsInitialized();

	/*
	* 슬롯 인덱스에 해당하는 위젯에 데이터를 반영하는 함수
	* @param SlotIndex: 스킬 슬롯
	* @param Data: HUD에서 스킬 슬롯을 표시하기 위해 복제되는 최소 데이터
	*/
	void UpdateSkillSlotWidget(const int32 SlotIndex, const FMSHUDSkillSlotData* Data);

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

	// 팀원 스킬 슬롯 위젯 (패시브)
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Transient)
	TObjectPtr<class UHorizontalBox> PassiveSkillListWidget;

	// 팀원 스킬 슬롯 위젯 (액티브)
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Transient)
	TObjectPtr<class UHorizontalBox> ActiveSkillListWidget;
	/* ======================== BindWidget ======================== */

	// 스킬 슬롯 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Skill")
	TSubclassOf<class UMSSkillSlotWidget> SkillSlotWidgetClass;

	// 팀원 스킬 슬롯 위젯 목록
	UPROPERTY(Transient)
	TArray<TObjectPtr<class UMSSkillSlotWidget>> TeamSkillWidgets;

private:
	// 슬롯 초기화 완료 여부
	bool bSkillSlotsInitialized = false;
};
