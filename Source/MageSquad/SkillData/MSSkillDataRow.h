#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MSSkillDataRow.generated.h"

USTRUCT(BlueprintType)
struct FMSSkillDataRow : public FTableRowBase
{
    GENERATED_BODY();

public:

    // SkillID
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SkillID = 0;

    // SkillName
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SkillName;

    // Skill level (1~10)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SkillLevel = 1;

    // 설명
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SkillDescription;

    // 피해량
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SkillDamage = 0.f;

    // 쿨타임
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CoolTime = 0.f;

    // 투사체 개수
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ProjectileNumber = 0;

    // 관통 횟수
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Penetration = 0;

    // 지속시간
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 0.f;

    // 스킬 범위
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Range = 0.f;
};
