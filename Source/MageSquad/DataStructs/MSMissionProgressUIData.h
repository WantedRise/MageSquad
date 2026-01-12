
#pragma once

#include "CoreMinimal.h"
#include "Types/GameMissionTypes.h"
#include "MSMissionProgressUIData.generated.h"

USTRUCT(BlueprintType)
struct FMSMissionProgressUIData
{
    GENERATED_BODY()

    UPROPERTY()
    EMissionType MissionType;

    // 공통
    UPROPERTY()
    float Normalized = 0.f;

    // Kill / Boss
    UPROPERTY()
    int32 CurrentHp = 0;

    UPROPERTY()
    int32 MaxHp = 0;

    // Find
    UPROPERTY()
    int32 CurrentCount = 0;

    UPROPERTY()
    int32 TargetCount = 0;
};