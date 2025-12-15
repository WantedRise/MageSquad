// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MSLobbyGameState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(
    FOnReadyTimeChanged,
    int32 /* RemainingSeconds */
);

UENUM(BlueprintType)
enum class ELobbyReadyPhase : uint8
{
    NotReady        UMETA(DisplayName = "Not Ready"),
    PartialReady    UMETA(DisplayName = "Partial Ready (60s)"),
    AllReady        UMETA(DisplayName = "All Ready (3s)")
};

/**
 * 
 */
UCLASS()
class MAGESQUAD_API AMSLobbyGameState : public AGameStateBase
{
	GENERATED_BODY()
public:
    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
public:
    FOnReadyTimeChanged OnReadyTimeChanged;

    void StartReadyCountdown(int32 StartSeconds);
protected:
    UFUNCTION()
    void OnRep_RemainingTime();
    void OnReadyCountdownFinished();
    void TickReadyCountdown();
protected:
    UPROPERTY(ReplicatedUsing = OnRep_RemainingTime)
    int32 RemainingReadyTime; // 남은 초

    UPROPERTY(Replicated)
    ELobbyReadyPhase LobbyReadyPhase;

    FTimerHandle ReadyTimerHandle;
};
