#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MSMissionFindData.generated.h"

/*
 * 작성자 : 이상준
 * 작성일 : 2026/01/11
 * 찾기미션 데이터 정보
 */

USTRUCT(BlueprintType)
struct FMSMissionFindData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Count;
};
