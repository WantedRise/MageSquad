#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "MSAutoActiveSkillListRow.generated.h"
/**
 * 작성자: 박세찬
 * 작성일: 25/12/22
 *
 * 스킬들의 데이터를 가지고 있는 데이터테이블을 만들기 위한 구조체
 */
USTRUCT(BlueprintType)
struct FMSAutoActiveSkillListRow : public FTableRowBase
{
    GENERATED_BODY();

public:
    // Skill ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SkillID = 0;

    // Skill Name
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SkillName;

    // Skill Event Tag
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag SkillEventTag;
};
