// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/MSGameModeBase.h"
#include "System/MSProjectilePoolSystem.h"
#include "Actors/Projectile/MSBaseProjectile.h"
#include "System/MSEnemySpawnSubsystem.h"

void AMSGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	if (UMSProjectilePoolSystem* ProjectilePoolSystem = GetGameInstance()->GetSubsystem<UMSProjectilePoolSystem>())
	{
		// �߻�ü Ǯ�� �ý��� �ʱ�ȭ
		// �⺻ �߻�ü Ŭ���� ���� + 200�� Ǯ��
		ProjectilePoolSystem->Configure(ProjectileClass, 200);
	}
	
	if (!GetWorld()->GetName().Contains(TEXT("Lvl_Dev_Lim")))
	{
		return;
	}

	if (UMSEnemySpawnSubsystem* SpawnSystem = UMSEnemySpawnSubsystem::Get(this))
	{
		// 설정
		SpawnSystem->SetSpawnInterval(2.0f);
		SpawnSystem->SetMaxActiveMonsters(100);
		SpawnSystem->SetSpawnRadius(3000.0f);
        
		// 스폰 시작
		SpawnSystem->StartSpawning();
	}
}
