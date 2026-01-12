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



/*
* 작성자: 이상준
* 작성일: 2025-12-16
* 로비에서 서버가 계산한 준비 상태 결과를
* 모든 클라이언트에게 동기화하고 전달하는 GameState 클래스.
*
* - 플레이어들의 준비 상태를 종합한 로비 준비 단계 관리
*   (미준비 / 일부 준비 / 전체 준비)
* - 준비 단계에 따른 카운트다운 시간 관리 및 동기화
* - 변경된 상태를 델리게이트를 통해 UI 계층에 전달
* 
* 준비 역할 분리:
* - 시간 카운트다운은 GameState에서 관리
* - 플레이어 준비 여부는 PlayerState에서 관리
* - UI는 상태 변경 이벤트만 수신하여 표시만 담당
*/
UCLASS()
class MAGESQUAD_API AMSLobbyGameState : public AGameStateBase
{
	GENERATED_BODY()
public:
    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
public:
    //준비 카운트다운 시간 변경 시 클라이언트에 알림
    FOnReadyTimeChanged OnReadyTimeChanged;
    //로비 준비 단계 변경 시 클라이언트에 알림
    FOnLobbyReadyPhaseChanged OnLobbyReadyPhaseChanged;
    //준비 카운트다운 중지
    void StopReadyCountdown();
    //준비 카운트다운 시작 
    void StartReadyCountdown(int32 StartSeconds);
    //로비 준비 단계 설정
    void SetLobbyReadyPhase(const ELobbyReadyPhase NewLobbyReadyPhase) {
        LobbyReadyPhase = NewLobbyReadyPhase;
    }
    //현재 로비 준비 단계 반환
    ELobbyReadyPhase GetLobbyReadyPhase() const { return LobbyReadyPhase; }
    //클라이언트에 로딩창 띄우기
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_ShowLoadingScreen();
    //OnLobbyReadyPhaseChanged Broadcast
    UFUNCTION()
    void OnRep_LobbyReadyPhase();
protected:
    //OnReadyTimeChanged Broadcast
    UFUNCTION()
    void OnRep_RemainingTime();
    //(서버 전용) 준비 카운트다운 완료시 AMSLobbyGameMode에 알림
    void OnReadyCountdownFinished();
    //카운트다운 타이머처리
    void TickReadyCountdown();
protected:
    //남은 준비 시간
    UPROPERTY(ReplicatedUsing = OnRep_RemainingTime)
    int32 RemainingReadyTime;
    //현재 로비 준비 단계
    UPROPERTY(ReplicatedUsing = OnRep_LobbyReadyPhase)
    ELobbyReadyPhase LobbyReadyPhase;
    //준비 카운트다운용 타이머 핸들
    FTimerHandle ReadyTimerHandle;
};
