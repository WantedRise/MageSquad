// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFlow/MSGameFlowPvE.h"
#include "GameStates/MSGameState.h"
#include "MageSquad.h"
#include "Components/MSGameProgressComponent.h"
#include "System/MSLevelManagerSubsystem.h"
#include "GameModes/MSGameMode.h"
#include "TimerManager.h"

void UMSGameFlowPvE::BeginDestroy()
{
	Super::BeginDestroy();

	if (UWorld* World = GetWorld())
	{
		// 타이머 해제 로직
		for (auto Handle : MissionTimerHandles)
		{
			GetWorld()->GetTimerManager().ClearTimer(Handle);
		}
	}
}

void UMSGameFlowPvE::Initialize(class AMSGameState* InOwnerGameState, UDataTable* InTimelineTable)
{
	Super::Initialize(InOwnerGameState, InTimelineTable);
	UE_LOG(LogMSNetwork, Log, TEXT("UMSGameFlowPvE Initialize Begin"));
	GameState->OnMissionFinished.AddUObject(this, &UMSGameFlowPvE::OnMissionFinished);

	static const FString Context(TEXT("PvEGameFlow"));
	MissionTimelineTable->GetAllRows(Context, MissionTimelineRows);
	
	CurrentMissionIndex = 0;
}

void UMSGameFlowPvE::OnEnterState(EGameFlowState NewState)
{
	switch (NewState)
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
	//SetState(EGameFlowState::Playing);

	for (const FMissionTimelineRow* Row : MissionTimelineRows)
	{
		if (!Row)
			continue;
		MissionTriggerTime += Row->TriggerTime;
		if (Row->MissionID == INDEX_NONE)
		{
			//랜덤 실행 //중복방지
		}
		else
		{
			ScheduleMission(MissionTriggerTime, Row->MissionID);
		}
	}

	if (GameProgress && MissionTimelineRows.Num() > 0)
	{
		GameProgress->Initialize(MissionTriggerTime);
	}

	if (GameProgress)
	{
		GameProgress->StartProgress();
	}
}

void UMSGameFlowPvE::ScheduleMission(float TriggerTime, int32 MissionID)
{
	if (!GameState)
		return;

	FTimerHandle Handle;

	TWeakObjectPtr<UMSGameFlowPvE> WeakThis(this);

	// AMSGameState.h에 선언되어 있다고 가정할 때
	FTimerDelegate MissionDelegate;
	MissionDelegate.BindUFunction(GameState, FName("SetCurrentMissionID"), MissionID);
	GameState->GetWorldTimerManager().SetTimer(Handle, MissionDelegate, TriggerTime, false);
	UE_LOG(LogTemp, Error, TEXT("ScheduleMission %f "), TriggerTime);
	MissionTimerHandles.Add(Handle);
}

void UMSGameFlowPvE::OnMissionFinished(int32 MissionId, bool bSuccess)
{
	if (bSuccess)
	{
		//보상처리 및 SetState Play로 전환
		UE_LOG(LogMSNetwork, Log, TEXT("UMSGameFlowPvE::OnMissionFinished mission success!! Give treasure"));
	}
	UE_LOG(LogMSNetwork, Log, TEXT("UMSGameFlowPvE::OnMissionFinished mission Fail!! Give treasure"));
}

void UMSGameFlowPvE::OnTimeCheckpoint()
{
	//호스트 로딩창 띄우기
	if (UMSLevelManagerSubsystem* LevelManager = GameState->GetGameInstance()->GetSubsystem<UMSLevelManagerSubsystem>())
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