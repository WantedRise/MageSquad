// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MSMissionTimeline.generated.h"

/*
 * 작성자 : 이상준
 * 작성일 : 2025/12/21
 * 미션 타임라인
 */

USTRUCT(BlueprintType)
struct FMissionTimelineRow: public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    float TriggerTime;

    UPROPERTY(EditAnywhere)
    int32 MissionID;
};