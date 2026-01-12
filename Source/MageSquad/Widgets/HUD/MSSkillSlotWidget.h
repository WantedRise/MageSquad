// Copyright (c) 2025 MageSquad

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Player/MSHUDDataComponent.h"
#include "MSSkillSlotWidget.generated.h"

/*
 * 작성자: 김준형
 * 작성일: 26/01/05
 *
 * 개별 스킬 슬롯을 나타내는 위젯 클래스
 * - 아이콘 머티리얼을 동적으로 적용하여 쿨다운 진행률을 머티리얼 파라미터로 표시
 * - 스킬이 비어 있을 경우 아이콘과 레벨 텍스트를 숨김
 */
UCLASS()
class MAGESQUAD_API UMSSkillSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// 슬롯을 비어있는 상태로 설정하는 함수
	void SetEmpty();

	/*
	* 스킬 슬롯에 스킬 데이터를 바인딩하는 함수
	* @param Data : HUD 데이터 컴포넌트에서 전달된 스킬 슬롯 데이터
	*/
	void BindSkill(const FMSHUDSkillSlotData& Data);

	// 현재 쿨다운 진행률 설정 함수
	void SetCooldownPercent(float InPercent);

protected:
	// 아이콘 이미지 위젯
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Transient)
	TObjectPtr<class UImage> IconImageWidget;

	// 스킬 레벨 텍스트 위젯
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Transient)
	TObjectPtr<class UTextBlock> SkillLevelTextWidget;

private:
	// 아이콘 머티리얼의 동적 인스턴스
	UPROPERTY(Transient)
	TObjectPtr<class UMaterialInstanceDynamic> IconMID;
};