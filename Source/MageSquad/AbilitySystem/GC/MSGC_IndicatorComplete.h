// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "MSGC_IndicatorComplete.generated.h"

/**
 * 작성자 : 임희섭
 * 작성일 : 2026/01/04
 * 인디케이터의 Fill이 다 찼을 때 인디케이터 위치에 발생시킬 GameplayCue
 */
UCLASS()
class MAGESQUAD_API UMSGC_IndicatorComplete : public UGameplayCueNotify_Static
{
	GENERATED_BODY()
	
public:
	UMSGC_IndicatorComplete();
		
protected:
	virtual bool OnExecute_Implementation(
		AActor* Target,
		const FGameplayCueParameters& Parameters) const override;
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TObjectPtr<UParticleSystem> CompleteParticle;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TObjectPtr<USoundBase> CompleteSound;
};
