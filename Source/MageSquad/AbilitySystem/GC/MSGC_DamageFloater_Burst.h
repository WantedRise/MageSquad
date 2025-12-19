// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Burst.h"
#include "MSGC_DamageFloater_Burst.generated.h"

/**
 * 작성자: 김준형
 * 작성일: 25/12/19
 *
 * 체력 변화량만큼 로컬 UI로 대미지 플로터 위젯을 그리는 GameplayCue
 */
UCLASS()
class MAGESQUAD_API UMSGC_DamageFloater_Burst : public UGameplayCueNotify_Burst
{
	GENERATED_BODY()

public:
	UMSGC_DamageFloater_Burst();

protected:
	virtual bool OnExecute_Implementation(AActor* Target, const FGameplayCueParameters& Parameters) const override;

	// 대미지 플로터 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Cue")
	TSubclassOf<class UMSDamageFloaterWidget> DamageFloaterWidget;

	// 치명타 여부 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Cue")
	FGameplayTag CriticalTag;

	// 액터 머리 위로 올릴 월드 Z 오프셋
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Cue")
	float WorldZOffset = 120.f;

	// 스크린에서 살짝 위에 뜨게 하는 오프셋(음수면 위)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Cue")
	FVector2D ScreenOffset = FVector2D(0.f, -20.f);
};
