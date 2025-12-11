#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MSIceSpearSkillDataRow.generated.h"

/**
 * 아이스 스피어 스킬 레벨별 데이터
 * 에디터에서 이 구조체를 RowStruct 로 사용하는 데이터테이블 작성
 */
USTRUCT(BlueprintType)
struct FMSIceSpearSkillDataRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 스킬 레벨
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SkillLevel = 1;

	// UI에 출력할 설명 텍스트
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SkillDescription;

	// 투사체 개수
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ProjectileNumber = 1;

	// 관통 횟수
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Penetration = 0;

	// 데미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SkillDamage = 20.f;
};
