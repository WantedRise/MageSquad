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
    virtual void Initialize(UWorld* World) override;
    virtual void Deinitialize() override;

    // 남은 오염 비율 (1 = 전부 더러움, 0 = 전부 정화)
    virtual float GetProgress() const override;

    virtual bool IsCompleted() const override
    {
        return GetProgress() <= 0.0f;
    }

private:
    void BindInkAreas(UWorld* World);
    void OnInkAreaUpdated();

private:
    // 정화 대상 영역들
    TArray<TWeakObjectPtr<class AMSInkAreaActor>> InkAreas;

    // 캐시된 진행도
    float Progress = 1.f;

    // (선택) 타임 리밋
    float TimeLimit = -1.f;
};