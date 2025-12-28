// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MSBossSpawnCutsceneWidget.generated.h"

/**
 * 작성자 : 임희섭
 * 작성일 : 2025/12/28
 * 보스 스폰 시 애니메이션을 실행할 위젯
 */
UCLASS()
class MAGESQUAD_API UMSBossSpawnCutsceneWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleBossSpawnEvent(bool bStart);
	
private:
	void TryBindToGameState();
	
private:
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<class UWidgetAnimation> CutSceneAnim;
	
};
