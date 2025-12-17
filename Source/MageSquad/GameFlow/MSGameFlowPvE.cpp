// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFlow/MSGameFlowPvE.h"
#include "GameStates/MSGameState.h"
#include "MageSquad.h"


void UMSGameFlowPvE::Initialize(AMSGameState* InOwnerGameState)
{
	Super::Initialize(InOwnerGameState);
	UE_LOG(LogMSNetwork, Log, TEXT("UMSGameFlowPvE Initialize Begin"));
	
	bIsBossDefeated = false;
	SetState(EGameFlowState::None);
}


void UMSGameFlowPvE::TickFlow(float DeltaSeconds)
{

}

void UMSGameFlowPvE::RegisterTimeEvent(EGameFlowState EventType,float TriggerTime)
{
	if (!OwnerGameState) return;

	FTimerHandle Handle;

	FTimerDelegate Delegate;
	Delegate.BindUObject(this, &UMSGameFlowPvE::OnGameEvent, EventType);

	OwnerGameState->GetWorld()->GetTimerManager().SetTimer(Handle, Delegate, TriggerTime, false);
	ScheduledEventHandles.Add(Handle);
}

void UMSGameFlowPvE::OnGameEvent(EGameFlowState InEventType)
{
	switch (InEventType)
	{
	case EGameFlowState::None:
		break;
	case EGameFlowState::Playing:
		//OwnerGameState->몬스터 스폰너에게 요청
		break;
	case EGameFlowState::Mission:
		TriggerRandomMission();
		break;
	case EGameFlowState::Boss:
		OwnerGameState->RequestSpawnFinalBoss();
		break;
	case EGameFlowState::Finished:
		break;
	default:
		break;
	}
}

void UMSGameFlowPvE::HandleMissionFinished(int32 MissionId, bool bSuccess)
{
	if (bSuccess)
	{
		//보상처리 및 SetState Play로 전환
	}
}

int32 UMSGameFlowPvE::SelectRandomMissionId()
{
	return 0;
}

void UMSGameFlowPvE::TriggerRandomMission()
{
	int32 MissionId = SelectRandomMissionId();
	//OwnerGameState->GetMissionComponent()->StartMission(MissionId);
}


void UMSGameFlowPvE::EnterNone()
{
}

void UMSGameFlowPvE::EnterPlaying()
{
}

void UMSGameFlowPvE::EnterQuesting()
{
}

void UMSGameFlowPvE::EnterBoss()
{
}

