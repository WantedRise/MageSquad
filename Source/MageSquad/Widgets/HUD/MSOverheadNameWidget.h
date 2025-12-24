// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MSOverheadNameWidget.generated.h"

/**
 * 작성자: 김준형
 * 작성일: 25/12/24
 *
 * 플레이어 머리 위 자기 이름을 표시하는 위젯
 * 자신의 이름은 자신에게 안 보임 (팀원에게만 자신의 이름을 노출)
 */
UCLASS()
class MAGESQUAD_API UMSOverheadNameWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 이름 텍스트 설정 함수
	UFUNCTION(BlueprintCallable)
	void SetNameText(const FText& InText);

protected:
	// 이름 텍스트 위젯
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Transient)
	TObjectPtr<class UTextBlock> NameText = nullptr;
};
