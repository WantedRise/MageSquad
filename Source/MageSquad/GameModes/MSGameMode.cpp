// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/MSGameMode.h"
#include "System/MSProjectilePoolSystem.h"
#include "Actors/Projectile/MSBaseProjectile.h"
#include "Enemy/MSNormalEnemy.h"
#include "Player/MSPlayerCharacter.h"
#include "System/MSEnemySpawnSubsystem.h"
#include "GameFlow/MSGameFlowBase.h"

void AMSGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (!GetWorld()->GetName().Contains(TEXT("Lvl_Dev_Lim")))
	{
		return;
	}

	if (UMSEnemySpawnSubsystem* SpawnSystem = GetWorld()->GetSubsystem<UMSEnemySpawnSubsystem>())
	{
		// 설정
		SpawnSystem->SetSpawnInterval(2.0f);
		SpawnSystem->SetMaxActiveMonsters(10);
		SpawnSystem->SetSpawnRadius(1500.0f);

		// 스폰 시작
		SpawnSystem->StartSpawning();
	}
}

TSubclassOf<UMSGameFlowBase> AMSGameMode::GetGameFlowClass() const
{
	return GameFlowClass;
}

