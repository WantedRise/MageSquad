// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Types/LobbyReadyTypes.h"
#include "MSLobbyGameState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(
    FOnReadyTimeChanged,
    int32 /* RemainingSeconds */
);

DECLARE_MULTICAST_DELEGATE_OneParam(
    FOnLobbyReadyPhaseChanged,
    ELobbyReadyPhase /* ELobbyReadyPhase */
);



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
    FOnLobbyReadyPhaseChanged OnLobbyReadyPhaseChanged;

    void StopReadyCountdown();
    void StartReadyCountdown(int32 StartSeconds);
    void SetLobbyReadyPhase(const ELobbyReadyPhase NewLobbyReadyPhase) {
        LobbyReadyPhase = NewLobbyReadyPhase;
    }
    ELobbyReadyPhase GetLobbyReadyPhase() const { return LobbyReadyPhase; }
    UFUNCTION()
    void OnRep_LobbyReadyPhase();
protected:
    UFUNCTION()
    void OnRep_RemainingTime();
    void OnReadyCountdownFinished();
    void TickReadyCountdown();
protected:
    UPROPERTY(ReplicatedUsing = OnRep_RemainingTime)
    int32 RemainingReadyTime; // 남은 초

    UPROPERTY(ReplicatedUsing = OnRep_LobbyReadyPhase)
    ELobbyReadyPhase LobbyReadyPhase;

    FTimerHandle ReadyTimerHandle;
};
