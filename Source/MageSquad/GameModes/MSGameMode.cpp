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

void AMSGameMode::BeginPlay()
{
	Super::BeginPlay();

	// if (!GetWorld()->GetName().Contains(TEXT("Lvl_Dev_Lim")))
	// {
	// 	return;
	// }

	if (UMSEnemySpawnSubsystem* SpawnSystem = GetWorld()->GetSubsystem<UMSEnemySpawnSubsystem>())
	{
		// 풀링 초기화
		SpawnSystem->InitializePool();
		
		// 설정
		SpawnSystem->SetSpawnInterval(2.0f);
		SpawnSystem->SetMaxActiveMonsters(10);
		SpawnSystem->SetSpawnRadius(1500.0f);
		SpawnSystem->SetSpawnCountPerTick(10);

		// 스폰 시작
		SpawnSystem->StartSpawning();
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

	// 전원 준비 완료
	if (AMSGameState* GS = GetGameState<AMSGameState>())
	{
		GS->StartGame();
	}
}


