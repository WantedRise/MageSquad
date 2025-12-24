// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Burst.h"
#include "MSGC_LevelUpFloater_Burst.generated.h"

/**
 * 작성자: 김준형
 * 작성일: 25/12/23
 *
 * 레벨업 시, 레벨업 VFX와 레벨업 플로터 위젯을 그리는 GameplayCue
 */
UCLASS()
class MAGESQUAD_API UMSGC_LevelUpFloater_Burst : public UGameplayCueNotify_Burst
{
	GENERATED_BODY()

public:
	UMSGC_LevelUpFloater_Burst();

protected:
	virtual bool OnExecute_Implementation(AActor* Target, const FGameplayCueParameters& Parameters) const override;

	// 레벨업 플로터 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Cue")
	TSubclassOf<class UMSLevelUpFloaterWidget> LevelUpFloaterWidget;

	// 액터 머리 위로 올릴 월드 Z 오프셋
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Cue")
	float WorldZOffset = 50.f;

	// 레벨업 나이아가라
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | VFX")
	TObjectPtr<class UNiagaraSystem> LevelUpNiagara;
};
