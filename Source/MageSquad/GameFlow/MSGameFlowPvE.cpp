// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFlow/MSGameFlowPvE.h"
#include "GameStates/MSGameState.h"
#include "MageSquad.h"
#include "Components/MSGameProgressComponent.h"
#include "System/MSLevelManagerSubsystem.h"
#include "GameModes/MSGameMode.h"
#include "TimerManager.h"
#include <System/MSMissionDataSubsystem.h>
#include "Actors/Wave/MSWaveManager.h"

UMSGameFlowPvE::UMSGameFlowPvE()
{
	static ConstructorHelpers::FClassFinder<AMSWaveManager>
		WaveManagerBP(TEXT("/Game/Blueprints/Actors/Wave/BP_WaveManager.BP_WaveManager_C"));

	if (WaveManagerBP.Succeeded())
	{
		WaveManagerClass = WaveManagerBP.Class;
	}
}

void UMSGameFlowPvE::BeginDestroy()
{
	Super::BeginDestroy();

	if (!GameState) return;
	if (UWorld* World = GameState->GetWorld())
	{
		// 타이머 해제 로직
		for (auto Handle : MissionTimerHandles)
		{
			World->GetTimerManager().ClearTimer(Handle);
		}
		World->GetTimerManager().ClearTimer(SkillLevelUpDelayTimerHandle);
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

	if (!WaveManagerClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("WaveManagerClass not set"));
		return;
	}
	
	FActorSpawnParameters Params;
	Params.Owner = nullptr;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	WaveManager = InOwnerGameState->GetWorld()->SpawnActor<AMSWaveManager>(
		WaveManagerClass,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		Params
	);

	InOwnerGameState->OnBossSpawnCutsceneStateChanged.AddUObject(WaveManager, &AMSWaveManager::SetWavePaused);
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

	SetWave(5.0f, 0.0f, MissionTriggerTime);
}

void UMSGameFlowPvE::SetWave(float FirstWaveTime,float DelayWaveTime, float EndTime)
{
	if (!WaveManager || !IsValid(WaveManager))
	{
		return;
	}

	FTimerHandle WaveStartDelayTimer;
	FTimerDelegate StartWaveDelegate;
	StartWaveDelegate.BindUObject(
		this,
		&UMSGameFlowPvE::StartWave,
		DelayWaveTime,
		EndTime
	);

	GetWorld()->GetTimerManager().SetTimer(
		WaveStartDelayTimer,
		StartWaveDelegate,
		FirstWaveTime,
		false
	);
}

void UMSGameFlowPvE::StartWave(float DelayWaveTime, float EndTime)
{
	if (!WaveManager || !IsValid(WaveManager))
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[GameFlow] Wave start"));

	WaveManager->StartWaveTimer(DelayWaveTime, EndTime);
}

void UMSGameFlowPvE::StopWave()
{
	if (!WaveManager || !IsValid(WaveManager))
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[GameFlow] Wave auto-stopped"));

	WaveManager->StopWaveTimer();
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
		UMSMissionDataSubsystem* MissionDataSubsystem = GameState->GetGameInstance()->GetSubsystem<UMSMissionDataSubsystem>();
		if (!MissionDataSubsystem) return;
		const FMSMissionRow* MissionData = MissionDataSubsystem->Find(MissionId);
		if (!MissionData) return;
		if (MissionData->MissionType == EMissionType::Boss) return;

		GiveMissionReward(MissionId);
		
		//보상처리 및 SetState Play로 전환
		UE_LOG(LogMSNetwork, Log, TEXT("UMSGameFlowPvE::OnMissionFinished mission success!! Give treasure"));
	}
	UE_LOG(LogMSNetwork, Log, TEXT("UMSGameFlowPvE::OnMissionFinished mission Fail!! Give treasure"));
}
void UMSGameFlowPvE::GiveMissionReward(int32 MissionId)
{
	if (!GameState || !GameState->HasAuthority())
		return;

	UMSMissionDataSubsystem* MissionDataSubsystem =
		GameState->GetGameInstance()->GetSubsystem<UMSMissionDataSubsystem>();
	if (!MissionDataSubsystem)
		return;

	const FMSMissionRow* MissionData = MissionDataSubsystem->Find(MissionId);
	if (!MissionData)
		return;

	// 보스 미션은 레벨업 페이즈 없음
	if (MissionData->MissionType == EMissionType::Boss)
	{
		UE_LOG(LogMSNetwork, Log,
			TEXT("Mission %d is Boss. Skip Skill Level Up Phase"), MissionId);
		return;
	}

	UE_LOG(LogMSNetwork, Log,
		TEXT("Mission %d success. Give reward & enter Skill Level Up Phase"),
		MissionId);

	// TODO: 골드, 아이템, 포인트 등 보상 처리 위치
	// GiveGold(MissionData->RewardGold);
	// GiveItem(MissionData->RewardItemId);

		// 중복 예약 방지
	if (GameState->GetWorld()->GetTimerManager().IsTimerActive(SkillLevelUpDelayTimerHandle))
		return;

	GameState->GetWorld()->GetTimerManager().SetTimer(
		SkillLevelUpDelayTimerHandle,
		this,
		&UMSGameFlowPvE::StartSkillLevelUpPhaseDelayed,
		1.5f,
		false
	);
}

void UMSGameFlowPvE::StartSkillLevelUpPhaseDelayed()
{
	if (!GameState || !GameState->HasAuthority())
		return;



	GameState->StartSkillLevelUpPhase(true);
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