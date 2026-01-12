
#pragma once

#include "CoreMinimal.h"
#include "LobbyReadyTypes.generated.h"

//로비 준비 단계
UENUM(BlueprintType)
enum class ELobbyReadyPhase : uint8
{
    NotReady        UMETA(DisplayName = "Not Ready"),
    PartialReady    UMETA(DisplayName = "Partial Ready (60s)"),
    AllReady        UMETA(DisplayName = "All Ready (3s)")
};
