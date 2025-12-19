// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFlow/MSGameFlowPvE.h"
#include "GameStates/MSGameState.h"
#include "MageSquad.h"
#include "Components/MSGameProgressComponent.h"
#include "System/MSLevelManagerSubsystem.h"
#include "GameModes/MSGameMode.h"
void UMSGameFlowPvE::Initialize(class AMSGameState* InOwnerGameState, float InTotalGameTime)
{
	Super::Initialize(InOwnerGameState, InTotalGameTime);
	UE_LOG(LogMSNetwork, Log, TEXT("UMSGameFlowPvE Initialize Begin"));
	
	bIsBossDefeated = false;
	if (GameProgress)
	{
		GameProgress->OnGameTimeReached.AddUObject(this, &UMSGameFlowPvE::OnTimeCheckpoint);
	}
}

void UMSGameFlowPvE::OnEnterState(EGameFlowState NewState)
{
	switch (NewState)
	{
	case EGameFlowState::None:
		break;
	case EGameFlowState::Playing:
		GameProgress->StartProgress();
		break;
	case EGameFlowState::Mission:
		break;
	case EGameFlowState::Boss:
		break;
	case EGameFlowState::Finished:
		break;
	default:
		break;
	}
}

void UMSGameFlowPvE::OnExitState(EGameFlowState OldState)
{
	switch (OldState)
	{
	case EGameFlowState::None:
		break;
	case EGameFlowState::Playing:
		break;
	case EGameFlowState::Mission:
		break;
	case EGameFlowState::Boss:
		break;
	case EGameFlowState::Finished:
		break;
	default:
		break;
	}
}
void UMSGameFlowPvE::Start()
{
	SetState(EGameFlowState::Playing);
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

void UMSGameFlowPvE::OnTimeCheckpoint()
{
	//호스트 로딩창 띄우기
	if (UMSLevelManagerSubsystem* LevelManager = OwnerGameState->GetGameInstance()->GetSubsystem<UMSLevelManagerSubsystem>())
	{
		//LevelManager->ShowLoadingWidget();
		LevelManager->HostGameAndTravelToLobby();
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

