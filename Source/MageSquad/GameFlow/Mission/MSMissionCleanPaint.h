// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFlow/Mission/MSMissionScript.h"
#include "MSMissionCleanPaint.generated.h"


struct FMSMissionProgressUIData;
struct FMSMissionCleanData;
/*
* 작성자: 이상준
* 작성일: 2025-12-30
*
* 오염 영역을 정화하는 미션 스크립트
*
* - 여러 개의 InkAreaActor를 스폰하여 정화 대상 영역을 구성
* - 각 영역의 정화 비율(CleanRatio)을 수집하여 전체 미션 진행도를 계산
* - 모든 영역이 일정 기준 이상 정화되면 미션 완료
*
*/
UCLASS()
class MAGESQUAD_API UMSMissionCleanPaint : public UMSMissionScript
{
    GENERATED_BODY()

public:
    UMSMissionCleanPaint();
    // 정화 대상 영역(InkAreaActor) 스폰 및 바인딩
    virtual void Initialize(UWorld* World) override;
    
    // 스폰된 영역 제거
    virtual void Deinitialize() override;

    // 남은 오염 비율 (1 = 전부 더러움, 0 = 전부 정화)
    virtual void GetProgress(FMSMissionProgressUIData& OutData) const override;

    virtual bool IsCompleted() const override;


private:
    // 정화 영역의 진행도 변경 시 호출되는 콜백
    void OnAreaProgressChanged(float);

    void RecalculateCurrentPercent();
    /** 타겟 생성 */
    void SpawnTargets(UWorld* World);
    void SetTargetInfo(TSubclassOf<AActor> InTargetClass);
    FVector CalculateAreaScaleFromSize(float SizeCM);
    TArray<int32> GetRandomIndicesLarge(int32 MaxIndex, int32 Count);
private:
    UPROPERTY()
    TSubclassOf<class AMSInkAreaActor> MissionActorClass;

    

    UPROPERTY(EditDefaultsOnly, Category = "Mission|CleanPaint")
    float TargetPercent = 0.94f;

    float CurrentPercent = 0.0f;


    UPROPERTY()
    class UDataTable* CleanMissionDataTable = nullptr;
    const FMSMissionCleanData* CleanData;

    UPROPERTY(EditDefaultsOnly, Category = "Mission | Clean")
    TSubclassOf<AActor> TargetActorClass;
    // 정화 대상 영역들
    UPROPERTY()
    TArray<TWeakObjectPtr<class AMSInkAreaActor>> InkAreas;
};