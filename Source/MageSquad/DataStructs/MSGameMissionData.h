// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Types/GameMissionTypes.h"
#include "MSGameMissionData.generated.h"

/*
 * 작성자 : 이상준
 * 작성일 : 2025/12/17
 * 미션 데이터 정보
 */

USTRUCT(BlueprintType)
struct FMissionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    int32 MissionId;

    UPROPERTY(EditAnywhere)
    EMissionType MissionType;

    UPROPERTY(EditAnywhere)
    float TimeLimit;

    UPROPERTY(EditAnywhere)
    int32 TargetCount;

    UPROPERTY(EditAnywhere)
    TSubclassOf<AActor> TargetActorClass;
};
