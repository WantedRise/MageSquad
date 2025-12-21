// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFlow/Mission/MSMissionScript.h"
#include "MSMissionEliteKillScript.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UMSMissionEliteKillScript : public UMSMissionScript
{
	GENERATED_BODY()
	
public:
    virtual void Initialize(UWorld* World) override;
    virtual void Deinitialize() override;
    virtual float GetProgress() const override;

private:
    void SpawnElite(UWorld* World);
    UFUNCTION()
    void OnEliteHPChanged(float CurrentHP, float MaxHP);

private:
    TWeakObjectPtr<class AActor> EliteMonster;
    float Progress = 0.f;
    float TimeLimit = -1.f;
};
