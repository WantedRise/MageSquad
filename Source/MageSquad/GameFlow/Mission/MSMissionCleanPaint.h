// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFlow/Mission/MSMissionScript.h"
#include "MSMissionCleanPaint.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UMSMissionCleanPaint : public UMSMissionScript
{
    GENERATED_BODY()

public:
    UMSMissionCleanPaint();
    virtual void Initialize(UWorld* World) override;
    virtual void Deinitialize() override;

    // 남은 오염 비율 (1 = 전부 더러움, 0 = 전부 정화)
    virtual float GetProgress() const override;

    virtual bool IsCompleted() const override
    {
        return GetProgress() >= 1.0f;
    }

private:
    void OnAreaProgressChanged(float);

private:
    UPROPERTY()
    TSubclassOf<class AMSInkAreaActor> MissionActorClass;

    // 정화 대상 영역들
    UPROPERTY()
    TArray<TWeakObjectPtr<class AMSInkAreaActor>> InkAreas;

    UPROPERTY(EditDefaultsOnly, Category = "Mission|CleanPaint")
    float AreaCompleteThreshold = 0.94f;
};