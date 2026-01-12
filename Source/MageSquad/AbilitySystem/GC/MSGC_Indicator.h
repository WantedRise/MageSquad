// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "MSGC_Indicator.generated.h"

/**
 * 작성자 : 임희섭
 * 작성일 : 2025/12/30
 * 
 */
UCLASS()
class MAGESQUAD_API AMSGC_Indicator : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()
	
public:
	AMSGC_Indicator();
	
protected:
	virtual bool OnExecute_Implementation(AActor* Target, const FGameplayCueParameters& Parameters) override;
	virtual bool OnRemove_Implementation(AActor* Target, const FGameplayCueParameters& Parameters) override;

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AMSIndicatorActor> IndicatorActorClass;
};
