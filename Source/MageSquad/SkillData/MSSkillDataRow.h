#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MSSkillDataRow.generated.h"
/**
 * 작성자: 박세찬
 * 작성일: 25/12/12
 * 
 * 스킬들의 데이터를 가지고 있는 데이터테이블을 만들기 위한 구조체
 */
USTRUCT(BlueprintType)
struct FMSSkillDataRow : public FTableRowBase
{
    GENERATED_BODY();

public:

    // Skill Type (자동 발동 액티브 : 1, 좌클릭 발동 액티브 : 2, 우클릭 발동 액티브 : 3)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SkillType = 1;

    // Skill ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SkillID = 0;

    // Skill Name
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SkillName;

    // Skill Event Tag
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag SkillEventTag;

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
