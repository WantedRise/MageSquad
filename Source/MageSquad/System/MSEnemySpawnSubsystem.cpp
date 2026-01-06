// Fill out your copyright notice in the Description page of Project Settings.

#include "System/MSEnemySpawnSubsystem.h"
#include "Enemy/MSBaseEnemy.h"
#include "Enemy/MSNormalEnemy.h"
#include "Enemy/MSEliteEnemy.h"
#include "Enemy/MSBossEnemy.h"
#include "DataStructs/MSEnemyStaticData.h"
#include "DataAssets/Enemy/DA_MonsterAnimationSetData.h"
#include "AbilitySystem/AttributeSets/MSEnemyAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Animation/Enemy/MSEnemyAnimInstance.h"
#include "Enemy/AIController/MSBaseAIController.h"
#include "Engine/World.h"
#include "Actors/TileMap/MSSpawnTileMap.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UMSEnemySpawnSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	// Commandlet(패키징, 데이터 처리 등) 환경이면 초기화 중단
	// 패키징 에러 처리
	if (IsRunningCommandlet())
	{
		return;
	}
	
	if (GetWorld()->GetName().Contains(TEXT("LobbyLevel")) || GetWorld()->GetName().Contains(TEXT("MainmenuLevel")))
	{
		return;
	}
	
	// DataTable 로드 및 에셋 사전 로딩
	LoadMonsterDataTable();
}

void UMSEnemySpawnSubsystem::Deinitialize()
{
	// 스폰 중지 및 타이머 정리
	StopSpawning();

	// 모든 활성 몬스터 제거
	ClearAllMonsters();

	// 풀의 모든 Enemy 액터 제거
	auto DestroyPoolEnemies = [](FMSEnemyPool& Pool)
	{
		for (AMSBaseEnemy* Enemy : Pool.FreeEnemies)
		{
			if (IsValid(Enemy))
			{
				Enemy->Destroy();
			}
		}
		Pool.FreeEnemies.Empty();
		Pool.ActiveEnemies.Empty();
	};

	DestroyPoolEnemies(NormalEnemyPool);
	DestroyPoolEnemies(EliteEnemyPool);
	DestroyPoolEnemies(BossEnemyPool);

	// 캐시 정리
	CachedMonsterData.Empty();
	CachedNormalMonsterData.Empty();
	CachedEliteMonsterData.Empty();
	CachedBossMonsterData.Empty();
	MonsterPoolMap.Empty();
	EnemyToPoolMap.Empty();

	UE_LOG(LogTemp, Log, TEXT("[MonsterSpawn] Subsystem Deinitialized"));

	Super::Deinitialize();
}

void UMSEnemySpawnSubsystem::InitializePool()
{
	// Commandlet(패키징, 데이터 처리 등) 환경이면 초기화 중단
	if (IsRunningCommandlet())
	{
		return;
	}
	
	if (GetWorld()->GetName().Contains(TEXT("LobbyLevel")) || GetWorld()->GetName().Contains(TEXT("MainmenuLevel")))
	{
		return;
	}

	if (GetWorld()->WorldType != EWorldType::PIE && GetWorld()->WorldType != EWorldType::Game)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SpawnSystem] Not PIE/Game world, skipping initialization"));
		return;
	}

	// NavSystem 참조 획득
	NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	
	// 풀 사전 생성 -> 서버에서만
	if (HasAuthority())
	{
		PrewarmPools();
	}

	UE_LOG(LogTemp, Log, TEXT("[MonsterSpawn] Subsystem Initialized - Server: %s"),
		   HasAuthority() ? TEXT("YES") : TEXT("NO"));
}

void UMSEnemySpawnSubsystem::LoadMonsterDataTable()
{
	if (!MonsterStaticDataTable)
	{
		MonsterStaticDataTable = LoadObject<UDataTable>(nullptr,
		                                                TEXT("/Game/Data/Enemy/DT/DT_MonsterStaticData"));

		if (!MonsterStaticDataTable)
		{
			return;
		}
	}

	TArray<FName> RowNames = MonsterStaticDataTable->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		FMSEnemyStaticData* RowData = MonsterStaticDataTable->FindRow<FMSEnemyStaticData>(RowName, TEXT(""));
		if (!RowData)
		{
			continue;
		}
		
		// 캐시된 데이터 생성 (값 복사)
		FMSCachedEnemyData& CachedData = CachedMonsterData.Add(RowName);

		// 에셋 동기 로딩 (초기화 단계에서만 수행)
		if (!RowData->SkeletalMesh.IsNull())
		{
			CachedData.SkeletalMesh = RowData->SkeletalMesh.LoadSynchronous();
			
			if (!RowData->Phase2SkeletalMesh.IsNull())
			{
				CachedData.Phase2SkeletalMesh = RowData->Phase2SkeletalMesh.LoadSynchronous();
			}
		}

		if (!RowData->AnimationSet.IsNull())
		{
			CachedData.AnimationSet = RowData->AnimationSet.LoadSynchronous();
		}

		// 스탯 복사
		CachedData.MaxHealth = RowData->MaxHealth;
		CachedData.MoveSpeed = RowData->MoveSpeed;
		CachedData.AttackDamage = RowData->AttackDamage;
		CachedData.AttackRange = RowData->AttackRange;
		CachedData.bIsRanged = RowData->bIsRanged;
		CachedData.ProjectileDataClass = RowData->ProjectileDataClass;
		CachedData.DropExpValue = RowData->DropExpValue;
		// GAS 데이터 복사
		if (!RowData->EnemyAbilities.IsNull())
		{
			CachedData.EnemyAbilities = RowData->EnemyAbilities.LoadSynchronous();
		}

		// 풀 매핑
		AssignMonsterToPool(RowName);
	}

	UE_LOG(LogTemp, Log, TEXT("[MonsterSpawn] Loaded %d monster types from DataTable"), CachedMonsterData.Num());
}

void UMSEnemySpawnSubsystem::AssignMonsterToPool(const FName& RowName)
{
	FString RowString = RowName.ToString();

	if (RowString.StartsWith(TEXT("Normal_")))
	{
		MonsterPoolMap.Add(RowName, &NormalEnemyPool);
		CachedNormalMonsterData.Add(RowName, CachedMonsterData[RowName]);
	}
	else if (RowString.StartsWith(TEXT("Elite_")))
	{
		MonsterPoolMap.Add(RowName, &EliteEnemyPool);
		CachedEliteMonsterData.Add(RowName, CachedMonsterData[RowName]);
	}
	else if (RowString.StartsWith(TEXT("Boss_")))
	{
		MonsterPoolMap.Add(RowName, &BossEnemyPool);
		CachedBossMonsterData.Add(RowName, CachedMonsterData[RowName]);
	}
	else
	{
		// 기본값: Normal 풀
		MonsterPoolMap.Add(RowName, &NormalEnemyPool);
		UE_LOG(LogTemp, Warning, TEXT("[MonsterSpawn] Unknown prefix for %s, assigned to Normal pool"), *RowString);
	}
}

void UMSEnemySpawnSubsystem::PrewarmPools()
{
	// 풀 클래스 설정
	NormalEnemyPool.EnemyClass = AMSNormalEnemy::StaticClass();
	NormalEnemyPool.InitialPoolSize = NormalEnemyPoolSize;

	EliteEnemyPool.EnemyClass = AMSEliteEnemy::StaticClass();
	EliteEnemyPool.InitialPoolSize = EliteEnemyPoolSize;

	BossEnemyPool.EnemyClass = AMSBossEnemy::StaticClass();
	BossEnemyPool.InitialPoolSize = BossEnemyPoolSize;

	// 각 풀 사전 생성
	PrewarmPool(NormalEnemyPool);
	PrewarmPool(EliteEnemyPool);
	PrewarmPool(BossEnemyPool);
}

void UMSEnemySpawnSubsystem::PrewarmPool(FMSEnemyPool& Pool)
{
	UWorld* World = GetWorld();
	if (!World || !Pool.EnemyClass)
	{
		return;
	}

	for (int32 i = 0; i < Pool.InitialPoolSize; ++i)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AMSBaseEnemy* Enemy = World->SpawnActor<AMSBaseEnemy>(
			Pool.EnemyClass,
			FVector(0, 0, 100.0f),
			FRotator::ZeroRotator,
			Params
		);

		if (Enemy)
		{
			//  풀링 모드 설정
			Enemy->SetPoolingMode(true);
			// 풀링된 Enemy는 DORM_Initial로 설정하여 클라이언트에 리플리케이트되지 않도록 함
			Enemy->SetNetDormancy(DORM_Initial);
			DeactivateEnemy(Enemy);
			Pool.FreeEnemies.Add(Enemy);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[MonsterSpawn] Prewarmed pool: %s (%d enemies)"),
	       *Pool.EnemyClass->GetName(), Pool.InitialPoolSize);
}

void UMSEnemySpawnSubsystem::StartSpawning()
{
	// 서버에서만 실행
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[MonsterSpawn] StartSpawning called on client - ignored"));
		return;
	}

	if (bIsSpawning)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MonsterSpawn] Already spawning"));
		return;
	}

	if (CachedMonsterData.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[MonsterSpawn] Cannot start: No monster data loaded!"));
		return;
	}

	bIsSpawning = true;

	GetWorld()->GetTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&UMSEnemySpawnSubsystem::SpawnMonsterTick,
		SpawnInterval,
		true // Loop
	);

	UE_LOG(LogTemp, Log, TEXT("[MonsterSpawn] Spawning started - Interval: %.2fs, Max: %d"),
	       SpawnInterval, MaxActiveMonsters);
}

void UMSEnemySpawnSubsystem::StopSpawning()
{
	if (!bIsSpawning)
	{
		return;
	}

	bIsSpawning = false;

	if (SpawnTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
		SpawnTimerHandle.Invalidate();
	}

	UE_LOG(LogTemp, Log, TEXT("[MonsterSpawn] Spawning stopped"));
}

void UMSEnemySpawnSubsystem::ClearAllMonsters()
{
	// 모든 활성 Enemy를 수집
	TArray<AMSBaseEnemy*> AllActiveEnemies;
	AllActiveEnemies.Append(NormalEnemyPool.ActiveEnemies);
	AllActiveEnemies.Append(EliteEnemyPool.ActiveEnemies);
	AllActiveEnemies.Append(BossEnemyPool.ActiveEnemies);

	// 타이머 정리 및 Destroy
	for (AMSBaseEnemy* Enemy : AllActiveEnemies)
	{
		if (IsValid(Enemy))
		{
			// 델리게이트 언바인딩
			UnbindEnemyDeathEvent(Enemy);

			// 타이머 정리 (풀 반환 타이머가 있을 수 있음)
			GetWorld()->GetTimerManager().ClearAllTimersForObject(Enemy);

			Enemy->Destroy();
		}
	}

	// 풀 정리
	NormalEnemyPool.ActiveEnemies.Empty();
	EliteEnemyPool.ActiveEnemies.Empty();
	BossEnemyPool.ActiveEnemies.Empty();

	// 역참조 맵 정리
	EnemyToPoolMap.Empty();

	CurrentActiveCount = 0;

	UE_LOG(LogTemp, Log, TEXT("[MonsterSpawn] Cleared all active monsters"));
}

AMSBaseEnemy* UMSEnemySpawnSubsystem::SpawnMonsterByID(const FName& MonsterID, const FVector& Location)
{
	// 서버에서만 실행
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[MonsterSpawn] SpawnMonsterByID called on client - ignored"));
		return nullptr;
	}

	return SpawnMonsterInternal(MonsterID, Location);
}

int32 UMSEnemySpawnSubsystem::GetScaledMaxActiveMonsters() const
{
	int32 PlayerCount = 0;

	if (UWorld* World = GetWorld())
	{
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			if (It->Get())
			{
				++PlayerCount;
			}
		}
	}

	// 최소 1명
	PlayerCount = FMath::Max(1, PlayerCount);

	return MaxActiveMonsters * PlayerCount;
}

void UMSEnemySpawnSubsystem::SpawnMonsterTick()
{
	// 서버 권한 체크
	if (!HasAuthority())
	{
		return;
	}

	TArray<APlayerController*> AllPlayers = GetAllPlayerControllers();
	if (AllPlayers.Num() == 0)
	{
		return;
	}

	// 인원수에 따른 최대 몬스터 수 스케일링
	const int32 ScaledMaxMonsters = GetScaledMaxActiveMonsters();

	// 최대 수량 체크
	if (CurrentActiveCount >= ScaledMaxMonsters)
	{
		return;
	}

	// 각 플레이어별로 스폰
	for (APlayerController* PC : AllPlayers)
	{
		if (!PC || !PC->GetPawn())
		{
			continue;
		}

		// 플레이어당 SpawnCountPerTick만큼 스폰
		for (int32 i = 0; i < SpawnCountPerTick; ++i)
		{
			// 최대 수량 재체크
			if (CurrentActiveCount >= ScaledMaxMonsters)
			{
				return;
			}

			// 랜덤 몬스터 타입 선택
			TArray<FName> AllMonsterTypes;
			CachedNormalMonsterData.GetKeys(AllMonsterTypes);

			if (AllMonsterTypes.Num() == 0)
			{
				return;
			}

			const FName SelectedMonsterID = AllMonsterTypes[FMath::RandRange(0, AllMonsterTypes.Num() - 1)];

			// 해당 플레이어 기준으로 스폰 위치 검색
			FVector SpawnLocation;
			if (!GetRandomSpawnLocation(PC, SpawnLocation))
			{
				continue; // 이 플레이어는 스킵하고 다음 플레이어 진행
			}

			// 스폰
			SpawnMonsterInternal(SelectedMonsterID, SpawnLocation);
		}
	}
}

AMSBaseEnemy* UMSEnemySpawnSubsystem::SpawnMonsterInternal(const FName& MonsterID, const FVector& Location)
{
	if (!HasAuthority())
	{
		return nullptr;
	}

	// 풀 찾기
	FMSEnemyPool** PoolPtr = MonsterPoolMap.Find(MonsterID);
	if (!PoolPtr || !*PoolPtr)
	{
		UE_LOG(LogTemp, Error, TEXT("[MonsterSpawn] No pool found for: %s"), *MonsterID.ToString());
		return nullptr;
	}

	FMSEnemyPool* Pool = *PoolPtr;

	// 풀에서 유효한 Enemy 가져오기
	AMSBaseEnemy* Enemy = nullptr;
	while (Pool->FreeEnemies.Num() > 0)
	{
		AMSBaseEnemy* Candidate = Pool->FreeEnemies.Pop();

		if (IsValid(Candidate))
		{
			Enemy = Candidate;
			break;
		}
	}

	// 풀이 비었으면 새로 생성
	if (!Enemy)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		Enemy = GetWorld()->SpawnActor<AMSBaseEnemy>(
			Pool->EnemyClass,
			Location,
			FRotator::ZeroRotator,
			Params
		);

		if (!Enemy)
		{
			UE_LOG(LogTemp, Error, TEXT("[MonsterSpawn] Failed to spawn new enemy!"));
			return nullptr;
		}

		UE_LOG(LogTemp, Log, TEXT("[MonsterSpawn] Pool exhausted, created new enemy (Type: %s)"),
		       *MonsterID.ToString());
	}


	// 몬스터 ID 설정
	Enemy->SetMonsterID(MonsterID);

	// DataTable 데이터로 초기화
	InitializeEnemyFromData(Enemy, MonsterID);
	
	// 활성화
	ActivateEnemy(Enemy, Location);

	// Active 풀에 추가
	Pool->ActiveEnemies.Add(Enemy);
	EnemyToPoolMap.Add(Enemy, Pool);

	++CurrentActiveCount;
	++TotalSpawnedCount;

	// 사망 이벤트 바인딩
	BindEnemyDeathEvent(Enemy);

	UE_LOG(LogTemp, Log, TEXT("[MonsterSpawn] Spawned: %s at %s (Active: %d)"),
	       *MonsterID.ToString(), *Location.ToString(), CurrentActiveCount);

	return Enemy;
}

bool UMSEnemySpawnSubsystem::GetRandomSpawnLocation(APlayerController* TargetPlayer, FVector& OutLocation)
{
	if (!TargetPlayer)
	{
		return false;
	}

	APawn* PlayerPawn = TargetPlayer->GetPawn();
	if (!PlayerPawn)
	{
		return false;
	}

	// 타일맵 시도
	if (AMSSpawnTileMap* TileMap = GetSpawnTileMap())
	{
		TArray<APlayerController*> AllPlayers = GetAllPlayerControllers();

		// 모든 플레이어에게 안 보이는 타일들 가져오기
		TArray<FMSSpawnTile> InvisibleTiles = TileMap->GetSpawnableTilesNotVisibleToPlayers(AllPlayers);

		if (InvisibleTiles.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("[MonsterSpawn] No invisible spawnable tiles"));
			return false;
		}

		const FVector PlayerLocation = PlayerPawn->GetActorLocation();

		// 사방에서 스폰되도록 4방향으로 분류
		TArray<FMSSpawnTile> NorthTiles;  // +Y
		TArray<FMSSpawnTile> SouthTiles;  // -Y
		TArray<FMSSpawnTile> EastTiles;   // +X
		TArray<FMSSpawnTile> WestTiles;   // -X

		for (const FMSSpawnTile& Tile : InvisibleTiles)
		{
			FVector ToTile = Tile.Location - PlayerLocation;
			
			// 주요 방향 판별
			if (FMath::Abs(ToTile.X) > FMath::Abs(ToTile.Y))
			{
				// 동서 방향
				if (ToTile.X > 0)
				{
					EastTiles.Add(Tile);
				}
				else
				{
					WestTiles.Add(Tile);
				}
			}
			else
			{
				// 남북 방향
				if (ToTile.Y > 0)
				{
					NorthTiles.Add(Tile);
				}
				else
				{
					SouthTiles.Add(Tile);
				}
			}
		}

		// 비어있지 않은 방향들 수집
		TArray<TArray<FMSSpawnTile>*> ValidDirections;
		if (NorthTiles.Num() > 0) ValidDirections.Add(&NorthTiles);
		if (SouthTiles.Num() > 0) ValidDirections.Add(&SouthTiles);
		if (EastTiles.Num() > 0) ValidDirections.Add(&EastTiles);
		if (WestTiles.Num() > 0) ValidDirections.Add(&WestTiles);

		if (ValidDirections.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("[MonsterSpawn] No valid directions"));
			return false;
		}

		// 랜덤 방향 선택
		TArray<FMSSpawnTile>* SelectedDirection = ValidDirections[FMath::RandRange(0, ValidDirections.Num() - 1)];

		// 선택된 방향에서 랜덤 타일 선택
		int32 RandomIndex = FMath::RandRange(0, SelectedDirection->Num() - 1);
		OutLocation = (*SelectedDirection)[RandomIndex].Location;
		return true;
	}

	// 타일맵이 없을때는 기존 NavMesh 방식 
	// 나중에 껐을 때 스폰이 제대로 이루어지지 않는 현상 테스트용으로 남겨둠

	if (!NavSystem)
	{
		NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
		if (!NavSystem)
		{
			return false;
		}
	}

	int32 ViewportSizeX, ViewportSizeY;
	TargetPlayer->GetViewportSize(ViewportSizeX, ViewportSizeY);

	constexpr int32 MaxAttempts = 30;

	for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
	{
		constexpr float OffScreenMargin = 1000.f;
		FVector2D ScreenEdgePoint = GetRandomScreenEdgePoint(ViewportSizeX, ViewportSizeY, OffScreenMargin);

		FVector WorldLocation, WorldDirection;
		if (TargetPlayer->DeprojectScreenPositionToWorld(ScreenEdgePoint.X, ScreenEdgePoint.Y, WorldLocation, WorldDirection))
		{
			const float SpawnDistance = FMath::FRandRange(4000.0f, 5000.0f);
			FVector CandidateLocation = WorldLocation + WorldDirection * SpawnDistance;
			CandidateLocation.Z = PlayerPawn->GetActorLocation().Z;

			// 다른 플레이어에게도 안 보이는지 체크
			if (IsLocationVisibleToAnyPlayer(CandidateLocation))
			{
				continue;
			}

			FNavLocation NavLoc;
			if (NavSystem->ProjectPointToNavigation(CandidateLocation, NavLoc, FVector(1000.0f, 1000.0f, 1000.0f)))
			{
				OutLocation = NavLoc.Location;
				OutLocation.Z = 92.f;
				return true;
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[MonsterSpawn] Failed to find valid spawn location after %d attempts"), MaxAttempts);
	return false;
}

// bool UMSEnemySpawnSubsystem::GetRandomSpawnLocation(FVector& OutLocation)
// {
// 	if (!NavSystem)
// 	{
// 		NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
// 		if (!NavSystem)
// 		{
// 			return false;
// 		}
// 	}
//
// 	// 플레이어 위치 기준
// 	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
// 	if (!PlayerPawn)
// 	{
// 		UE_LOG(LogTemp, Warning, TEXT("[MonsterSpawn] No player pawn found"));
// 		return false;
// 	}
//
// 	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
// 	if (!PC)
// 	{
// 		return false;
// 	}
//
// 	// 카메라 정보
// 	FVector CameraLocation;
// 	FRotator CameraRotation;
// 	PC->GetPlayerViewPoint(CameraLocation, CameraRotation);
//
// 	// 뷰포트 크기
// 	int32 ViewportSizeX, ViewportSizeY;
// 	PC->GetViewportSize(ViewportSizeX, ViewportSizeY);
//
// 	constexpr int32 MaxAttempts = 30; // 위치 찾는 시도 횟수
// 	constexpr float OffScreenMargin = 1000.f; // 화면 밖 여유 거리
//
// 	for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
// 	{
// 		// 화면 가장자리 4방향 중 랜덤 선택
// 		FVector2D ScreenEdgePoint = GetRandomScreenEdgePoint(ViewportSizeX, ViewportSizeY, OffScreenMargin);
//
// 		// 스크린 좌표를 월드 좌표로 역변환
// 		FVector WorldLocation, WorldDirection;
// 		if (PC->DeprojectScreenPositionToWorld(ScreenEdgePoint.X, ScreenEdgePoint.Y, WorldLocation, WorldDirection))
// 		{
// 			// 카메라에서 일정 거리만큼 떨어진 지점 계산
// 			const float SpawnDistance = FMath::FRandRange(4000.0f, 5000.0f);
// 			FVector CandidateLocation = WorldLocation + WorldDirection * SpawnDistance;
//
// 			// 플레이어 높이 기준으로 조정
// 			CandidateLocation.Z = PlayerPawn->GetActorLocation().Z;
//
// 			// NavMesh 검증
// 			FNavLocation NavLoc;
// 			if (NavSystem->ProjectPointToNavigation(CandidateLocation, NavLoc, FVector(1000.0f, 1000.0f, 1000.0f)))
// 			{
// 				OutLocation = NavLoc.Location;
// 				OutLocation.Z = 92.f;
// 				return true;
// 			}
// 		}
// 	}
//
// 	UE_LOG(LogTemp, Warning, TEXT("[MonsterSpawn] Failed to find valid spawn location after %d attempts"), MaxAttempts);
// 	return false;
// }

bool UMSEnemySpawnSubsystem::IsLocationVisibleToPlayer(const APlayerController* PC, const FVector& Location)
{
	if (!PC)
	{
		return false;
	}

	FVector CameraLocation;
	FRotator CameraRotation;
	PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

	// 스크린 좌표 변환
	FVector2D ScreenPosition;
	if (PC->ProjectWorldLocationToScreen(Location, ScreenPosition, false))
	{
		// 뷰포트 크기 획득
		int32 ViewportSizeX, ViewportSizeY;
		PC->GetViewportSize(ViewportSizeX, ViewportSizeY);

		// 화면 경계에 마진 추가 (선택적 - 완전히 화면 밖을 원할 경우)
		constexpr float Margin = 100.0f; // 픽셀 단위

		// 화면 내부에 있는지 체크
		if (ScreenPosition.X >= -Margin && ScreenPosition.X <= ViewportSizeX + Margin &&
			ScreenPosition.Y >= -Margin && ScreenPosition.Y <= ViewportSizeY + Margin)
		{
			return true; // 화면에 보임
		}
	}

	return false; // 화면 밖
}

bool UMSEnemySpawnSubsystem::IsLocationVisibleToAnyPlayer(const FVector& Location)
{
	// 모든 플레이어 컨트롤러 체크
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && IsLocationVisibleToPlayer(PC, Location))
		{
			return true; // 한 명이라도 보고 있으면 true
		}
	}

	return false; // 모든 플레이어 시야 밖
}

FVector2D UMSEnemySpawnSubsystem::GetRandomScreenEdgePoint(int32 ViewportSizeX, int32 ViewportSizeY,
                                                           float Margin)
{
	// 4개 가장자리 중 하나 선택: 0=상단, 1=하단, 2=좌측, 3=우측
	int32 Edge = FMath::RandRange(0, 3);

	FVector2D ScreenPoint;

	switch (Edge)
	{
	case 0: // 상단
		ScreenPoint.X = FMath::FRandRange(0.0f, ViewportSizeX);
		ScreenPoint.Y = -Margin;
		break;

	case 1: // 하단
		ScreenPoint.X = FMath::FRandRange(0.0f, ViewportSizeX);
		ScreenPoint.Y = ViewportSizeY + Margin;
		break;

	case 2: // 좌측
		ScreenPoint.X = -Margin;
		ScreenPoint.Y = FMath::FRandRange(0.0f, ViewportSizeY);
		break;

	case 3: // 우측
		ScreenPoint.X = ViewportSizeX + Margin;
		ScreenPoint.Y = FMath::FRandRange(0.0f, ViewportSizeY);
		break;
		
	default:
		break;
	}

	return ScreenPoint;
}

void UMSEnemySpawnSubsystem::InitializeEnemyFromData(AMSBaseEnemy* Enemy, const FName& MonsterID)
{
	if (!Enemy)
	{
		return;
	}

	// 캐시된 데이터 가져오기
	FMSCachedEnemyData* Data = CachedMonsterData.Find(MonsterID);
	if (!Data)
	{
		UE_LOG(LogTemp, Error, TEXT("[MonsterSpawn] Monster data not found: %s"), *MonsterID.ToString());
		return;
	}

	// 스켈레탈 메시 설정
	if (Data->SkeletalMesh)
	{
		if (Enemy->GetMesh()->IsRegistered())
		{
			Enemy->GetMesh()->UnregisterComponent();
		}

		Enemy->GetMesh()->SetSkeletalMesh(Data->SkeletalMesh);
		Enemy->SetPhase2SkeletalMesh(Data->Phase2SkeletalMesh);
		
		// 이전 스켈레탈메시의 머테리얼 정보가 남아있어 머테리얼이 이상하게 나타나던 현상 방지 코드
		const TArray<FSkeletalMaterial>& MeshMaterials = Data->SkeletalMesh->GetMaterials();
		for (int32 i = 0; i < MeshMaterials.Num(); ++i)
		{
			Enemy->GetMesh()->SetMaterial(i, MeshMaterials[i].MaterialInterface);
		}

		// 렌더링 상태 강제 업데이트 및 재등록
		Enemy->GetMesh()->RegisterComponent(); // 렌더링 시스템에 메시를 다시 등록
	
		UE_LOG(LogTemp, Error, TEXT("%s : EnemySetting : SkeletalMeshSet Success"), HasAuthority() ? TEXT("[SERVER]") : TEXT("[CLIENT]"));
	}

	// 애니메이션 설정
	if (Data->AnimationSet && Data->AnimationSet->AnimationClass)
	{
		Enemy->SetAnimData(Data->AnimationSet);
		UE_LOG(LogTemp, Error, TEXT("%s : EnemySetting : AnimDataSet Success"), HasAuthority() ? TEXT("[SERVER]") : TEXT("[CLIENT]"));
	}

	// GAS 속성 초기화
	// 서버에서만 GAS 초기화
	if (HasAuthority())
	{
		if (UAbilitySystemComponent* ASC = Enemy->GetAbilitySystemComponent())
		{
			const UMSEnemyAttributeSet* AttributeSet = Cast<UMSEnemyAttributeSet>(
				ASC->GetAttributeSet(UMSEnemyAttributeSet::StaticClass())
			);

			if (AttributeSet)
			{
				ASC->SetNumericAttributeBase(AttributeSet->GetMaxHealthAttribute(), Data->MaxHealth);
				ASC->SetNumericAttributeBase(AttributeSet->GetCurrentHealthAttribute(), Data->MaxHealth);
				ASC->SetNumericAttributeBase(AttributeSet->GetMoveSpeedAttribute(), Data->MoveSpeed);
				Enemy->GetCharacterMovement()->MaxWalkSpeed = Data->MoveSpeed;
				ASC->SetNumericAttributeBase(AttributeSet->GetAttackDamageAttribute(), Data->AttackDamage);
				ASC->SetNumericAttributeBase(AttributeSet->GetAttackRangeAttribute(), Data->AttackRange);
				ASC->SetNumericAttributeBase(AttributeSet->GetDropExpValueAttribute(), Data->DropExpValue);
			}
			
			if (Data->bIsRanged)
			{
				// 원거리 몬스터면 스킬 데이터 세팅
				Enemy->SetProjectileData(Data->ProjectileDataClass);
			}

			// 어빌리티 부여
			ASC->ClearAllAbilities();
			for (const TSubclassOf<UGameplayAbility>& AbilityClass : Data->EnemyAbilities->EnemyAbilities)
			{
				if (AbilityClass)
				{
					FGameplayAbilitySpec Spec(AbilityClass, 1, INDEX_NONE, Enemy);
					ASC->GiveAbility(Spec);
				}
			}
		}
	}
}

void UMSEnemySpawnSubsystem::ActivateEnemy(AMSBaseEnemy* Enemy, const FVector& Location) const
{
	if (!Enemy)
	{
		return;
	}

	// 풀링 모드 해제
	Enemy->SetPoolingMode(false);

	// 위치 설정
	if (Location != FVector())
	{
		Enemy->SetActorLocation(Location);
		Enemy->SetActorRotation(FRotator::ZeroRotator);
	}
	
	//  가시성/충돌 활성화
	Enemy->SetActorHiddenInGame(false);
	Enemy->SetActorEnableCollision(true);
	
	// 리플리케이션 활성화
	Enemy->SetReplicates(true);
	Enemy->SetReplicateMovement(true);
	Enemy->SetNetDormancy(DORM_Awake);
	Enemy->ForceNetUpdate();

	// 디버그 로그
	// UE_LOG(LogTemp, Error, TEXT("★★★ ActivateEnemy: %s | bReplicates: %d | Dormancy: %d ★★★"),
	// 	*Enemy->GetName(),
	// 	Enemy->GetIsReplicated(),
	// 	(int32)Enemy->NetDormancy
	// );

	//  RVO 재활성화
	if (UCharacterMovementComponent* MovementComp = Enemy->GetCharacterMovement())
	{
		// Velocity 초기화
		MovementComp->Velocity = FVector::ZeroVector;
		MovementComp->UpdateComponentVelocity();

		// RVO 재등록 (UID가 0이면 새로 할당됨)
		MovementComp->SetAvoidanceEnabled(false);
		MovementComp->SetAvoidanceEnabled(true);

		// RVO 파라미터 재설정 (혹시 모를 초기화 대비)
		MovementComp->bUseRVOAvoidance = true;
		MovementComp->AvoidanceConsiderationRadius = 500.0f;
		MovementComp->AvoidanceWeight = 0.5f;
		MovementComp->SetAvoidanceGroup(1);
		MovementComp->SetGroupsToAvoidMask(1);
	}

	// AI 컨트롤러 시작
	if (AController* Controller = Enemy->GetController())
	{
		if (AMSBaseAIController* AIController = Cast<AMSBaseAIController>(Controller))
		{
			AIController->RunAI();
		}
	}
	else
	{
		// 컨트롤러가 없으면 생성
		Enemy->SpawnDefaultController();
		UE_LOG(LogTemp, Error, TEXT("Spawn Default Controller"));
	}
}

void UMSEnemySpawnSubsystem::DeactivateEnemy(AMSBaseEnemy* Enemy)
{
	if (!Enemy)
	{
		return;
	}

	Enemy->SetMonsterID(NAME_None);
	
	// Hidden 처리
	Enemy->SetActorHiddenInGame(true);
	// 콜리전은 Dead Ability에서 몽타주 시작과 동시에 처리
	Enemy->SetActorTickEnabled(false);

	// Movement 정리
	if (UCharacterMovementComponent* MovementComp = Enemy->GetCharacterMovement())
	{
		MovementComp->StopMovementImmediately();
		MovementComp->Velocity = FVector::ZeroVector;
		
		// SetAvoidanceEnabled 내부의 ensure(GetCharacterOwner()) 통과를 위함
		if (MovementComp->GetCharacterOwner() != nullptr)
		{
			MovementComp->SetAvoidanceEnabled(false);
		}
		else 
		{
			// Owner가 없다면 RVO를 직접 끌 수 없으므로, bUseRVOAvoidance 변수를 직접 건드리는 방법도 있습니다.
			MovementComp->bUseRVOAvoidance = false;
		}
	}

	// AI 정지
	if (AController* Controller = Enemy->GetController())
	{
		if (AMSBaseAIController* AIController = Cast<AMSBaseAIController>(Controller))
		{
			AIController->StopAI();
		}
	}

	//  GAS 초기화
	ResetEnemyGASState(Enemy);
	
	// 리플리케이션 완전히 끄기
	Enemy->SetReplicates(false);
	Enemy->SetReplicateMovement(false);

	// UE_LOG(LogTemp, Warning, TEXT("DeactivateEnemy - AFTER: bReplicates: %d"),
	// 	Enemy->GetIsReplicated()
	// );
}

void UMSEnemySpawnSubsystem::ResetEnemyGASState(AMSBaseEnemy* Enemy)
{
	if (!Enemy)
	{
		return;
	}

	UAbilitySystemComponent* ASC = Enemy->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	// 모든 GameplayTag 제거
	FGameplayTagContainer AllTags;
	ASC->GetOwnedGameplayTags(AllTags);
	ASC->RemoveLooseGameplayTags(AllTags);

	// 모든 활성 GameplayEffect 제거
	ASC->RemoveActiveEffects(FGameplayEffectQuery());

	//  모든 Ability 취소
	ASC->CancelAllAbilities();
}

void UMSEnemySpawnSubsystem::BindEnemyDeathEvent(AMSBaseEnemy* Enemy)
{
	if (!Enemy)
	{
		return;
	}

	UAbilitySystemComponent* ASC = Enemy->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	// // "Enemy.State.Dead" 태그 변경 감지
	FGameplayTag DeathTag = FGameplayTag::RequestGameplayTag(FName("Enemy.State.Dead"));

	ASC->RegisterGameplayTagEvent(DeathTag, EGameplayTagEventType::NewOrRemoved)
	   .AddUObject(this, &UMSEnemySpawnSubsystem::OnEnemyDeathTagChanged, Enemy);
}

void UMSEnemySpawnSubsystem::UnbindEnemyDeathEvent(AMSBaseEnemy* Enemy)
{
	if (!Enemy)
	{
		return;
	}

	UAbilitySystemComponent* ASC = Enemy->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	// 모든 델리게이트 언바인딩
	FGameplayTag DeathTag = FGameplayTag::RequestGameplayTag(FName("Enemy.State.Dead"));
	ASC->RegisterGameplayTagEvent(DeathTag).RemoveAll(this);
}

void UMSEnemySpawnSubsystem::OnEnemyDeathTagChanged(const FGameplayTag Tag, int32 NewCount, AMSBaseEnemy* Enemy)
{
	if (NewCount >= 0) // 태그가 없음 == 몽타주 끝남
	{
		HandleEnemyDeath(Enemy);
	}
	
	else // 태그가 제거됨 = 몽타주 끝남 = 사망처리
	{
		//HandleEnemyDeath(Enemy);
	}
}

void UMSEnemySpawnSubsystem::HandleEnemyDeath(AMSBaseEnemy* Enemy)
{
	if (!Enemy)
	{
		return;
	}

	// 풀 찾기 (O(1))
	FMSEnemyPool* OwningPool = FindPoolForEnemy(Enemy);
	if (!OwningPool)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MonsterSpawn] Dead enemy not found in any pool!"));
		return;
	}

	// Active 풀에서 제거
	OwningPool->ActiveEnemies.Remove(Enemy);
	EnemyToPoolMap.Remove(Enemy);
	--CurrentActiveCount;

	// 델리게이트 언바인딩
	UnbindEnemyDeathEvent(Enemy);

	UE_LOG(LogTemp, Verbose, TEXT("[MonsterSpawn] Enemy died (Active: %d)"), CurrentActiveCount);

	// 사망 애니메이션 후 풀로 반환
	FTimerHandle ReturnTimer;
	GetWorld()->GetTimerManager().SetTimer(
		ReturnTimer,
		FTimerDelegate::CreateUObject(this, &UMSEnemySpawnSubsystem::ReturnEnemyToPoolInternal, Enemy, OwningPool),
		DeathAnimationDuration,
		false
	);
}

void UMSEnemySpawnSubsystem::ReturnEnemyToPool(AMSBaseEnemy* Enemy)
{
	if (!Enemy)
	{
		return;
	}

	FMSEnemyPool* OwningPool = FindPoolForEnemy(Enemy);
	if (!OwningPool)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MonsterSpawn] Cannot return enemy: pool not found"));
		return;
	}

	ReturnEnemyToPoolInternal(Enemy, OwningPool);
}

void UMSEnemySpawnSubsystem::ReturnEnemyToPoolInternal(AMSBaseEnemy* Enemy, FMSEnemyPool* Pool)
{
	if (!Enemy || !Pool)
	{
		return;
	}

	// GAS 상태 완전 초기화
	ResetEnemyGASState(Enemy);

	// 델리게이트 언바인딩 (중복 호출 방지)
	UnbindEnemyDeathEvent(Enemy);

	// 비활성화
	DeactivateEnemy(Enemy);

	// Free 풀에 추가
	Pool->FreeEnemies.Add(Enemy);

	UE_LOG(LogTemp, Verbose, TEXT("[MonsterSpawn] Enemy returned to pool (Free: %d, Active: %d)"),
	       Pool->FreeEnemies.Num(), Pool->ActiveEnemies.Num());
}

void UMSEnemySpawnSubsystem::SetSpawnInterval(float NewInterval)
{
	SpawnInterval = FMath::Max(0.1f, NewInterval);

	// 스폰 중이면 타이머 재시작
	if (bIsSpawning)
	{
		StopSpawning();
		StartSpawning();
	}
}

void UMSEnemySpawnSubsystem::SetMaxActiveMonsters(int32 NewMax)
{
	MaxActiveMonsters = FMath::Max(1, NewMax);
}

void UMSEnemySpawnSubsystem::SetSpawnRadius(float NewRadius)
{
	SpawnRadius = FMath::Max(500.0f, NewRadius);
}

void UMSEnemySpawnSubsystem::SetSpawnCountPerTick(int InSpawnCountPerTick)
{
	SpawnCountPerTick = FMath::Max(1, InSpawnCountPerTick);
}

void UMSEnemySpawnSubsystem::SetMonsterDataTable(UDataTable* NewDataTable)
{
	if (!NewDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MonsterSpawn] SetMonsterDataTable: null table!"));
		return;
	}

	// 기존 활성 몬스터 제거
	ClearAllMonsters();

	// 기존 풀의 Enemy들 제거
	auto DestroyPoolEnemies = [](FMSEnemyPool& Pool)
	{
		for (AMSBaseEnemy* Enemy : Pool.FreeEnemies)
		{
			if (IsValid(Enemy))
			{
				Enemy->Destroy();
			}
		}
		Pool.FreeEnemies.Empty();
	};

	DestroyPoolEnemies(NormalEnemyPool);
	DestroyPoolEnemies(EliteEnemyPool);
	DestroyPoolEnemies(BossEnemyPool);

	// 캐시 정리
	CachedMonsterData.Empty();
	CachedNormalMonsterData.Empty();
	CachedEliteMonsterData.Empty();
	CachedBossMonsterData.Empty();
	MonsterPoolMap.Empty();

	// 새 DataTable 할당
	MonsterStaticDataTable = NewDataTable;

	// 데이터 다시 로드
	LoadMonsterDataTable();

	// 풀 재생성
	PrewarmPools();

	UE_LOG(LogTemp, Log, TEXT("[MonsterSpawn] DataTable reloaded successfully"));
}

FMSEnemyPool* UMSEnemySpawnSubsystem::FindPoolForEnemy(AMSBaseEnemy* Enemy) const
{
	FMSEnemyPool* const* PoolPtr = EnemyToPoolMap.Find(Enemy);
	return PoolPtr ? *PoolPtr : nullptr;
}

bool UMSEnemySpawnSubsystem::HasAuthority() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	// 스탠드얼론이거나 서버인 경우 true
	ENetMode NetMode = World->GetNetMode();
	return NetMode == NM_Standalone || NetMode == NM_DedicatedServer || NetMode == NM_ListenServer;
}

AMSSpawnTileMap* UMSEnemySpawnSubsystem::GetSpawnTileMap()
{
	if (SpawnTileMap.IsValid())
	{
		return SpawnTileMap.Get();
	}

	for (TActorIterator<AMSSpawnTileMap> It(GetWorld()); It; ++It)
	{
		SpawnTileMap = *It;
		return SpawnTileMap.Get();
	}
	return nullptr;
}

TArray<APlayerController*> UMSEnemySpawnSubsystem::GetAllPlayerControllers() const
{
	TArray<APlayerController*> Result;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			Result.Add(PC);
		}
	}
	return Result;
}

UMSEnemySpawnSubsystem* UMSEnemySpawnSubsystem::Get(UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	return World->GetSubsystem<UMSEnemySpawnSubsystem>();
}
