
#pragma once

#include "CoreMinimal.h"
#include "GameMissionTypes.generated.h"

UENUM(BlueprintType)
enum class EMissionType : uint8
{
	KillElite  UMETA(DisplayName = "KillElite"),
	ClearPaint UMETA(DisplayName = "ClearPaint"),
};