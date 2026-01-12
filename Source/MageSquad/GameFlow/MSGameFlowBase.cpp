// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFlow/MSGameFlowBase.h"
#include "GameStates/MSGameState.h"
#include "MageSquad.h"
#include "Components/MSGameProgressComponent.h"

void UMSGameFlowBase::Start()
{
    
}

void UMSGameFlowBase::Initialize(AMSGameState* InGameState, UDataTable* InTimelineTable)
{
    check(InTimelineTable);
    MissionTimelineTable = InTimelineTable;
    check(InGameState);
    GameState = InGameState;
    GameProgress = GameState->FindComponentByClass<UMSGameProgressComponent>();
    check(GameProgress);
    
    CurrentState = EGameFlowState::None;
}

void UMSGameFlowBase::TickFlow(float DeltaSeconds)
{

}
void UMSGameFlowBase::OnEnterState(EGameFlowState NewState)
{

}

void UMSGameFlowBase::OnExitState(EGameFlowState OldState)
{

}
void UMSGameFlowBase::OnGameTimeReached(float ElapsedSeconds)
{

}
void UMSGameFlowBase::SetState(EGameFlowState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

    const EGameFlowState PrevState = CurrentState;

    //이전 상태 종료 처리
    OnExitState(PrevState);

    //상태 변경
    CurrentState = NewState;

    //새 상태 진입 처리
    OnEnterState(NewState);

    UE_LOG(LogMSNetwork, Log,
        TEXT("[GameFlow] State Changed: %s -> %s"),
        *UEnum::GetValueAsString(PrevState),
        *UEnum::GetValueAsString(NewState)
    );
}
