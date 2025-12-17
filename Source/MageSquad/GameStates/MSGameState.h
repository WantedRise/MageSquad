// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "MSGameState.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API AMSGameState : public AGameState
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;
public:
	float GetGameElapsedTime() const { return GameElapsedTime; }

	class UMSGameFlowBase* GetGameFlow() const { return GameFlow; }
protected:
	//UPROPERTY(Replicated)
	float GameElapsedTime;

	UPROPERTY()
	class UMSGameFlowBase* GameFlow;
};
