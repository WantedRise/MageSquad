// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MSGameFlowBase.h"
#include "MSGameFlowPvE.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UMSGameFlowPvE : public UMSGameFlowBase
{
	GENERATED_BODY()
	
public:
	virtual void TickFlow(float DeltaSeconds) override;

};
