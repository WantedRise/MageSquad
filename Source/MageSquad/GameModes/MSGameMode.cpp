// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/MSGameMode.h"
#include "System/MSProjectilePoolSystem.h"
#include "Actors/Projectile/MSBaseProjectile.h"
#include "Enemy/MSNormalEnemy.h"
#include "Player/MSPlayerCharacter.h"
#include "System/MSEnemySpawnSubsystem.h"
#include "GameFlow/MSGameFlowBase.h"
#include "GameStates/MSGameState.h"
#include "Player/MSPlayerState.h"
#include "MageSquad.h"
#include "Utils/MSUtils.h"
#include "System/MSLevelManagerSubsystem.h"
#include "System/MSMissionDataSubsystem.h"
#include <Player/MSPlayerController.h>
#include <System/MSCharacterDataSubsystem.h>
#include "OnlineSubsystemTypes.h"

AMSGameMode::AMSGameMode()
{
	bUseSeamlessTravel = true;
}

void AMSGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 공유 목숨이 0 + 팀 전멸 이벤트에 로비 이동 함수 바인딩
	GetGameState<AMSGameState>()->OnSharedLivesDepleted.AddLambda(
		[&]()
		{
			ExecuteTravelToLobby();
		}
	);
}

void AMSGameMode::SetupGameFlow()
{
	check(GameFlowClass);
	check(MissionTimelineTable);

	AMSGameState* GS = GetGameState<AMSGameState>();
	check(GS);
	GS->OnMissionFinished.AddUObject(this, &AMSGameMode::OnMissionFinished);

	//GameFlow 생성 (UObject, 서버 전용)
	GameFlow = NewObject<UMSGameFlowBase>(this, GameFlowClass);
	check(GameFlow);
	//DataTable + GameState 주입
	GameFlow->Initialize(GS, MissionTimelineTable);
}

void AMSGameMode::OnMissionFinished(int32 MissionID, bool bSuccess)
{
	auto* Subsystem = GetGameInstance()->GetSubsystem<UMSMissionDataSubsystem>();
	if (const FMSMissionRow* MissionData = Subsystem ? Subsystem->Find(MissionID) : nullptr)
	{
		if (MissionData->MissionType != EMissionType::Boss)
		{
			return;
		}
		FTimerHandle TimerHandle;
		FTimerDelegate TimerDelegate;

		TimerDelegate.BindUObject(this, &AMSGameMode::ExecuteTravelToLobby);
		GetWorldTimerManager().SetTimer(TimerHandle, TimerDelegate, 3.0f, false);
	}
}

void AMSGameMode::ExecuteTravelToLobby()
{
	if (auto* LevelManager = GetGameInstance()->GetSubsystem<UMSLevelManagerSubsystem>())
	{
		LevelManager->HostGameAndTravelToLobby();
	}
}


TSubclassOf<UMSGameFlowBase> AMSGameMode::GetGameFlowClass() const
{
	return GameFlowClass;
}


void AMSGameMode::TryStartGame()
{
	for (APlayerState* PS : GameState->PlayerArray)
	{
		AMSPlayerState* MSPS = Cast<AMSPlayerState>(PS);
		if (!MSPS)
		{
			MS_LOG(LogMSNetwork, Log, TEXT("%s"), TEXT("AMSPlayerState == null"));
			return;
		}
		else if (!MSPS->IsUIReady())
		{
			MS_LOG(LogMSNetwork, Log, TEXT("%s"), TEXT("AMSPlayerState IsUIReady"));
			return;
		}
	}

	if (nullptr == GameFlow)
	{
		SetupGameFlow();

		if (GameFlow)
		{
			GameFlow->Start();

			if (UMSEnemySpawnSubsystem* SpawnSystem = UMSEnemySpawnSubsystem::Get(GetWorld()))
			{
				// 오브젝트 풀링 시키기
				SpawnSystem->InitializePool();

				// 설정
				SpawnSystem->SetSpawnInterval(5.0f);
				SpawnSystem->SetMaxActiveMonsters(10);
				SpawnSystem->SetSpawnRadius(1500.0f);
				SpawnSystem->SetSpawnCountPerTick(10);

				// 5초 뒤 스폰 시작
				SpawnSystem->StartSpawning();
			}
		}
		else
		{
			MS_LOG(LogMSNetwork, Log, TEXT("%s"), TEXT("GameFlow == null"));
		}
	}
}

void AMSGameMode::NotifyClientsShowLoadingWidget()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (AMSPlayerController* PC = Cast<AMSPlayerController>(It->Get()))
		{
			// 각 클라이언트에게 로딩 위젯을 띄우라고 전송
			PC->ClientShowLoadingWidget();
		}
	}
}

void AMSGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
}

void AMSGameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);
}
