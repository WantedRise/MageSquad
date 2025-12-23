// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/MSGameMode.h"
#include "System/MSProjectilePoolSystem.h"
#include "Actors/Projectile/MSBaseProjectile.h"
#include "Enemy/MSNormalEnemy.h"
#include "Player/MSPlayerCharacter.h"
#include "System/MSEnemySpawnSubsystem.h"
#include "GameFlow/MSGameFlowBase.h"
#include "GameStates/MSGameState.h"
#include "System/MSSteamManagerSubsystem.h"
#include <Player/MSPlayerState.h>
#include "MageSquad.h"
#include "Utils/MSUtils.h"

void AMSGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void AMSGameMode::SetupGameFlow()
{
	check(GameFlowClass);
	check(MissionTimelineTable);

	AMSGameState* GS = GetGameState<AMSGameState>();
	check(GS);

	//GameFlow 생성 (UObject, 서버 전용)
	GameFlow = NewObject<UMSGameFlowBase>(this, GameFlowClass);
	check(GameFlow);
	//DataTable + GameState 주입
	GameFlow->Initialize(GS, MissionTimelineTable);
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
				SpawnSystem->SetSpawnInterval(2.0f);
				SpawnSystem->SetMaxActiveMonsters(10);
				SpawnSystem->SetSpawnRadius(1500.0f);
				SpawnSystem->SetSpawnCountPerTick(10);

				// 10초 뒤 스폰 시작
				SpawnSystem->StartSpawning();

				UE_LOG(LogTemp, Log, TEXT("[GameMode] 10 seconds delay finished. Spawning started!"));

				// Test Boss Spawn 코드 입니다. 이 코드로 보스 스폰 가능합니다.
				// SpawnSystem->SpawnMonsterByID(MSUtils::ENEMY_BOSS_FEY, FVector(0.f, 0.f,400.f));
			}
		}
		else
		{
			MS_LOG(LogMSNetwork, Log, TEXT("%s"), TEXT("GameFlow == null"));
		}
	}
}
