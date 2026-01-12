#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MSMissionCleanData.generated.h"

/*
 * 작성자 : 이상준
 * 작성일 : 2026/01/09
 * 정화미션 데이터 정보
 */

USTRUCT(BlueprintType)
struct FMSMissionCleanData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 AreaCount;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float AreaSize;
};
