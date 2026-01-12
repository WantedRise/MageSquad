// Copyright ...

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MSSharedExperienceData.generated.h"

 /**
  * 작성자: 김준형
  * 작성일: 25/12/22
  *
  * 레벨별 필요 경험치 테이블 Row
  * 플레이 인원(1~4)에 따라 동일 레벨이라도 요구 경험치가 달라지도록 데이터화
  */

USTRUCT(BlueprintType)
struct FMSSharedLevelExpRow : public FTableRowBase
{
	GENERATED_BODY()

	// 게임 플레이 인원 별 필요 경험치 변동값 반환 함수 (최대 4인)
	FORCEINLINE float GetRequiredEXPByPlayerCount(int32 PlayerCount) const
	{
		switch (FMath::Clamp(PlayerCount, 1, 4))
		{
		case 1: return RequiredEXP_1P;
		case 2: return RequiredEXP_2P;
		case 3: return RequiredEXP_3P;
		case 4: return RequiredEXP_4P;
		default: return RequiredEXP_4P;
		}
	}

	// 가독성을 위해 RowName과 동일하게 유지
	// (ex. Level = 1이면 RowName="1")
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RequiredEXP_1P = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RequiredEXP_2P = 140.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RequiredEXP_3P = 170.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RequiredEXP_4P = 200.f;
};
