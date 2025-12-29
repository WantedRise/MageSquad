// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFlow/Mission/MSMissionScript.h"
#include "GameplayEffectTypes.h"
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
    virtual bool IsCompleted() const override;
private:
    void SpawnElite(UWorld* World);

    void OnEliteHPChanged(const FOnAttributeChangeData& Data);

private:
    TWeakObjectPtr<class AMSBaseEnemy> EliteMonster;
    float Progress = 0.f;
    float TimeLimit = -1.f;
    float MaxHP;
};
