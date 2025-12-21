
#pragma once

#include "CoreMinimal.h"
#include "GameMissionTypes.generated.h"

UENUM(BlueprintType)
enum class EMissionType : uint8
{
	None			UMETA(DisplayName = "None"),
	EliteMonster    UMETA(DisplayName = "EliteMonster"),
	Boss			UMETA(DisplayName = "Boss"),
	ClearPaint		UMETA(DisplayName = "ClearPaint"),
};