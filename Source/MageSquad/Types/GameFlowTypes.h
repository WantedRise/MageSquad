
#pragma once

#include "CoreMinimal.h"
#include "GameFlowTypes.generated.h"

UENUM(BlueprintType)
enum class EGameFlowState : uint8
{
	None     UMETA(DisplayName = "None"),
	Playing  UMETA(DisplayName = "Playing"),
	Mission  UMETA(DisplayName = "Mission"),
	Boss     UMETA(DisplayName = "Boss"),
	Finished UMETA(DisplayName = "Finished"),
};