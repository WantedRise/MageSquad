// Fill out your copyright notice in the Description page of Project Settings.


#include "GameStates/GameFlow/MSGameFlowBase.h"

void UMSGameFlowBase::Initialize(AMSGameState* OwnerGS)
{
}

void UMSGameFlowBase::TickFlow(float DeltaSeconds)
{
}

void UMSGameFlowBase::SetState(EGameFlowState NewState)
{
    if (GameFlowState != NewState)
    {
        GameFlowState = NewState;
    }
}
