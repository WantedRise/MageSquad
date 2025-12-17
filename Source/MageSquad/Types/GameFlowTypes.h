
#pragma once

#include "CoreMinimal.h"
#include "GameFlowTypes.generated.h"

UENUM(BlueprintType)
enum class EGameFlowState : uint8
{
    None,
    Playing,
    Questing,
    Boss,
    Finished
};
