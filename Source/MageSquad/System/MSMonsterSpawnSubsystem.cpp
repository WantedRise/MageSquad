// Fill out your copyright notice in the Description page of Project Settings.

#include "System/MSMonsterSpawnSubsystem.h"
#include "Enemy/MSBaseEnemy.h"
#include "Enemy/MSNormalEnemy.h"
#include "Enemy/MSEliteEnemy.h"
#include "Enemy/MSBossEnemy.h"
#include "Enemy/AIController/MSBaseAIController.h"
#include "DataStructs/MSMonsterStaticData.h"
#include "DataAssets/Enemy/DA_MonsterAnimationSetData.h"
#include "AbilitySystem/AttributeSets/MSEnemyAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"

void UMSMonsterSpawnSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// NavSystem 참조 획득
	NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

	// DataTable 로드 및 캐싱
	LoadMonsterDataTable();

	// 풀 사전 생성
	PrewarmPools();

	UE_LOG(LogTemp, Log, TEXT("MSMonsterSpawnSubsystem Initialized"));
}

void UMSMonsterSpawnSubsystem::Deinitialize()
{
	StopSpawning();
	ClearAllMonsters();

	// 메모리 정리
	NormalEnemyPool.FreeEnemies.Empty();
	NormalEnemyPool.ActiveEnemies.Empty();
	EliteEnemyPool.FreeEnemies.Empty();
	EliteEnemyPool.ActiveEnemies.Empty();
	BossEnemyPool.FreeEnemies.Empty();
	BossEnemyPool.ActiveEnemies.Empty();
	CachedMonsterData.Empty();
	MonsterPoolMap.Empty();

	Super::Deinitialize();
}

void UMSMonsterSpawnSubsystem::LoadMonsterDataTable()
{
	if (!MonsterStaticDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("MonsterStaticDataTable is not assigned! Please set it in the subsystem settings."));
		return;
	}

	TArray<FName> RowNames = MonsterStaticDataTable->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		FMSMonsterStaticData* RowData = MonsterStaticDataTable->FindRow<FMSMonsterStaticData>(RowName, TEXT(""));
		if (RowData)
		{
			CachedMonsterData.Add(RowName, RowData);
			AssignMonsterToPool(RowName);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Loaded %d monster types from DataTable"), CachedMonsterData.Num());
}

void UMSMonsterSpawnSubsystem::AssignMonsterToPool(const FName& RowName)
{
	FString RowString = RowName.ToString();

	if (RowString.StartsWith("Normal_"))
	{
		MonsterPoolMap.Add(RowName, &NormalEnemyPool);
	}
	else if (RowString.StartsWith("Elite_"))
	{
		MonsterPoolMap.Add(RowName, &EliteEnemyPool);
	}
	else if (RowString.StartsWith("Boss_"))
	{
		MonsterPoolMap.Add(RowName, &BossEnemyPool);
	}
	else
	{
		// 기본값: Normal 풀에 할당
		MonsterPoolMap.Add(RowName, &NormalEnemyPool);
	}
}

void UMSMonsterSpawnSubsystem::PrewarmPools()
{
	NormalEnemyPool.EnemyClass = AMSNormalEnemy::StaticClass();
	NormalEnemyPool.InitialPoolSize = NormalEnemyPoolSize;

	EliteEnemyPool.EnemyClass = AMSEliteEnemy::StaticClass();
	EliteEnemyPool.InitialPoolSize = EliteEnemyPoolSize;

	BossEnemyPool.EnemyClass = AMSBossEnemy::StaticClass();
	BossEnemyPool.InitialPoolSize = BossEnemyPoolSize;

	PrewarmPool(NormalEnemyPool);
	PrewarmPool(EliteEnemyPool);
	PrewarmPool(BossEnemyPool);
}

void UMSMonsterSpawnSubsystem::PrewarmPool(FMSEnemyPool& Pool)
{
	UWorld* World = GetWorld();
	if (!World || !Pool.EnemyClass)
	{
		return;
	}

	for (int32 i = 0; i < Pool.InitialPoolSize; ++i)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AMSBaseEnemy* Enemy = World->SpawnActor<AMSBaseEnemy>(
			Pool.EnemyClass,
			FVector(0, 0, -10000),  // 맵 아래로 숨김
			FRotator::ZeroRotator,
			Params
		);

		if (Enemy)
		{
			DeactivateEnemy(Enemy);
			Pool.FreeEnemies.Add(Enemy);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Prewarmed pool with %d enemies of class %s"),
		Pool.InitialPoolSize, *Pool.EnemyClass->GetName());
}

void UMSMonsterSpawnSubsystem::StartSpawning()
{
	if (bIsSpawning)
	{
		UE_LOG(LogTemp, Warning, TEXT("Spawning already active"));
		return;
	}

	if (CachedMonsterData.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot start spawning: No monster data loaded!"));
		return;
	}

	bIsSpawning = true;
	CurrentWave = 1;
	CurrentDifficultyMultiplier = 1.0f;

	GetWorld()->GetTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&UMSMonsterSpawnSubsystem::SpawnMonsterTick,
		SpawnInterval,
		true  // Loop
	);

	UE_LOG(LogTemp, Log, TEXT("Monster spawning started - Interval: %.2fs, Max Active: %d"),
		SpawnInterval, MaxActiveMonsters);
}

void UMSMonsterSpawnSubsystem::StopSpawning()
{
	if (!bIsSpawning)
	{
		return;
	}

	bIsSpawning = false;
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);

	UE_LOG(LogTemp, Log, TEXT("Monster spawning stopped"));
}

void UMSMonsterSpawnSubsystem::ClearAllMonsters()
{
	// 모든 활성 적 제거
	TArray<AMSBaseEnemy*> AllActive;
	AllActive.Append(NormalEnemyPool.ActiveEnemies);
	AllActive.Append(EliteEnemyPool.ActiveEnemies);
	AllActive.Append(BossEnemyPool.ActiveEnemies);

	for (AMSBaseEnemy* Enemy : AllActive)
	{
		if (IsValid(Enemy))
		{
			Enemy->Destroy();
		}
	}

	NormalEnemyPool.ActiveEnemies.Empty();
	EliteEnemyPool.ActiveEnemies.Empty();
	BossEnemyPool.ActiveEnemies.Empty();

	CurrentActiveCount = 0;

	UE_LOG(LogTemp, Log, TEXT("Cleared all monsters"));
}

void UMSMonsterSpawnSubsystem::SpawnMonsterTick()
{
	if (CurrentActiveCount >= MaxActiveMonsters)
	{
		return;  // 최대 수량 도달
	}

	// 랜덤 몬스터 타입 선택
	TArray<FName> AllMonsterTypes;
	CachedMonsterData.GetKeys(AllMonsterTypes);

	if (AllMonsterTypes.Num() == 0)
	{
		return;
	}

	FName SelectedMonsterID = AllMonsterTypes[FMath::RandRange(0, AllMonsterTypes.Num() - 1)];
	SpawnMonster(SelectedMonsterID);
}

AMSBaseEnemy* UMSMonsterSpawnSubsystem::SpawnMonster(const FName& MonsterID)
{
	FMSEnemyPool** PoolPtr = MonsterPoolMap.Find(MonsterID);
	if (!PoolPtr || !*PoolPtr)
	{
		UE_LOG(LogTemp, Error, TEXT("No pool found for monster ID: %s"), *MonsterID.ToString());
		return nullptr;
	}

	FMSEnemyPool* Pool = *PoolPtr;

	FVector SpawnLocation;
	if (!GetRandomSpawnLocation(SpawnLocation))
	{
		return nullptr;
	}

	// 풀에서 가져오기 시도
	AMSBaseEnemy* Enemy = nullptr;
	while (Pool->FreeEnemies.Num() > 0 && !Enemy)
	{
		AMSBaseEnemy* Candidate = Pool->FreeEnemies.Pop();
		if (IsValid(Candidate))
		{
			Enemy = Candidate;
		}
	}

	// 풀이 비었으면 새로 생성
	if (!Enemy)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		Enemy = GetWorld()->SpawnActor<AMSBaseEnemy>(
			Pool->EnemyClass,
			SpawnLocation,
			FRotator::ZeroRotator,
			Params
		);

		if (!Enemy)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to spawn new enemy!"));
			return nullptr;
		}

		UE_LOG(LogTemp, Log, TEXT("Pool exhausted, spawned new enemy (Total pool size now: %d)"),
			Pool->FreeEnemies.Num() + Pool->ActiveEnemies.Num() + 1);
	}

	// DataTable 데이터로 초기화
	InitializeMonsterFromData(Enemy, MonsterID);

	// 활성화
	ActivateEnemy(Enemy, SpawnLocation);

	// 액티브 풀에 추가
	Pool->ActiveEnemies.Add(Enemy);
	CurrentActiveCount++;
	TotalSpawnedCount++;

	// 사망 이벤트 바인딩
	BindEnemyDeathEvent(Enemy);

	// 웨이브 업데이트
	UpdateDifficultyWave();

	// 이벤트 브로드캐스트
	OnMonsterSpawned.Broadcast(Enemy, MonsterID, CurrentWave);

	return Enemy;
}

bool UMSMonsterSpawnSubsystem::GetRandomSpawnLocation(FVector& OutLocation)
{
	if (!NavSystem)
	{
		UE_LOG(LogTemp, Error, TEXT("NavSystem is null!"));
		return false;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("No player pawn found"));
		return false;
	}

	FVector PlayerLocation = PlayerPawn->GetActorLocation();

	const int32 MaxAttempts = 10;
	for (int32 i = 0; i < MaxAttempts; ++i)
	{
		// MinSpawnDistance ~ SpawnRadius 범위에서 랜덤 위치 생성
		float Angle = FMath::FRandRange(0.0f, 360.0f);
		float Distance = FMath::FRandRange(MinSpawnDistance, SpawnRadius);

		FVector RandomOffset = FVector(
			FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
			FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
			0.0f
		);

		FVector TestLocation = PlayerLocation + RandomOffset;

		// NavMesh에 투영
		FNavLocation NavLocation;
		bool bFound = NavSystem->ProjectPointToNavigation(
			TestLocation,
			NavLocation,
			FVector(500.0f, 500.0f, 500.0f)  // Search extent
		);

		if (bFound)
		{
			OutLocation = NavLocation.Location;
			return true;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Failed to find valid spawn location after %d attempts"), MaxAttempts);
	return false;
}

void UMSMonsterSpawnSubsystem::InitializeMonsterFromData(AMSBaseEnemy* Enemy, const FName& MonsterID)
{
	if (!Enemy)
	{
		return;
	}

	FMSMonsterStaticData** DataPtr = CachedMonsterData.Find(MonsterID);
	if (!DataPtr || !*DataPtr)
	{
		UE_LOG(LogTemp, Error, TEXT("Monster data not found for ID: %s"), *MonsterID.ToString());
		return;
	}

	FMSMonsterStaticData* Data = *DataPtr;

	// 1. 스켈레탈 메시 설정
	if (!Data->SkeletalMesh.IsNull())
	{
		USkeletalMesh* Mesh = Data->SkeletalMesh.LoadSynchronous();
		if (Mesh)
		{
			Enemy->GetMesh()->SetSkeletalMesh(Mesh);
		}
	}

	// 2. 애니메이션 설정 (AnimationSet에서 AnimBP 로드)
	// 주의: DA_MSMonsterAnimationSet에는 AnimSequence만 있고 AnimBP가 없음
	// 필요시 AnimBP 프로퍼티를 추가하거나 다른 방식으로 설정

	// 3. 속성 초기화 (난이도 스케일링 적용)
	if (UAbilitySystemComponent* ASC = Enemy->GetAbilitySystemComponent())
	{
		if (TCopyQualifiersFromTo_T<const UAttributeSet, UMSEnemyAttributeSet>* AttributeSet = 
			Cast<UMSEnemyAttributeSet>(ASC->GetAttributeSet(UMSEnemyAttributeSet::StaticClass())))
		{
			float ScaledMaxHealth = Data->MaxHealth * CurrentDifficultyMultiplier;
			float ScaledMoveSpeed = Data->MoveSpeed * FMath::Min(CurrentDifficultyMultiplier, 1.5f);  // 속도는 150%까지만
			float ScaledAttackDamage = Data->AttackDamage * CurrentDifficultyMultiplier;

			ASC->SetNumericAttributeBase(AttributeSet->GetMaxHealthAttribute(), ScaledMaxHealth);
			ASC->SetNumericAttributeBase(AttributeSet->GetHealthAttribute(), ScaledMaxHealth);
			ASC->SetNumericAttributeBase(AttributeSet->GetMoveSpeedAttribute(), ScaledMoveSpeed);
			ASC->SetNumericAttributeBase(AttributeSet->GetAttackDamageAttribute(), ScaledAttackDamage);
			ASC->SetNumericAttributeBase(AttributeSet->GetAttackRangeAttribute(), Data->AttackRange);
		}
	}

	// 4. 어빌리티 부여
	if (UAbilitySystemComponent* ASC = Enemy->GetAbilitySystemComponent())
	{
		ASC->ClearAllAbilities();

		for (const TSubclassOf<UGameplayAbility>& AbilityClass : Data->StartAbilities)
		{
			if (AbilityClass)
			{
				FGameplayAbilitySpec Spec(AbilityClass, 1, INDEX_NONE, Enemy);
				ASC->GiveAbility(Spec);
			}
		}
	}

	// 5. GameplayEffect 적용
	if (UAbilitySystemComponent* ASC = Enemy->GetAbilitySystemComponent())
	{
		for (const TSubclassOf<UGameplayEffect>& EffectClass : Data->StartEffects)
		{
			if (EffectClass)
			{
				FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
				EffectContext.AddSourceObject(Enemy);
				FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EffectClass, 1, EffectContext);
				if (SpecHandle.IsValid())
				{
					ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				}
			}
		}
	}
}

void UMSMonsterSpawnSubsystem::ActivateEnemy(AMSBaseEnemy* Enemy, const FVector& Location)
{
	if (!Enemy)
	{
		return;
	}

	Enemy->SetActorLocation(Location);
	Enemy->SetActorRotation(FRotator::ZeroRotator);
	Enemy->SetActorHiddenInGame(false);
	Enemy->SetActorEnableCollision(true);
	Enemy->SetActorTickEnabled(true);

	// AI 재시작
	if (AController* Controller = Enemy->GetController())
	{
		if (AMSBaseAIController* AIController = Cast<AMSBaseAIController>(Controller))
		{
			AIController->RunAI();
		}
	}
	else
	{
		// 컨트롤러가 없으면 새로 생성
		Enemy->SpawnDefaultController();
	}
}

void UMSMonsterSpawnSubsystem::DeactivateEnemy(AMSBaseEnemy* Enemy)
{
	if (!Enemy)
	{
		return;
	}

	Enemy->SetActorHiddenInGame(true);
	Enemy->SetActorEnableCollision(false);
	Enemy->SetActorTickEnabled(false);
	Enemy->SetActorLocation(FVector(0, 0, -10000));

	if (AController* Controller = Enemy->GetController())
	{
		if (AMSBaseAIController* AIController = Cast<AMSBaseAIController>(Controller))
		{
			AIController->StopAI();
		}
	}
}

// ========================================================================================
// 사망 처리 및 풀 반환
// ========================================================================================

void UMSMonsterSpawnSubsystem::BindEnemyDeathEvent(AMSBaseEnemy* Enemy)
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

	// "State.Dead" 태그 변경 감지
	FGameplayTag DeathTag = FGameplayTag::RequestGameplayTag(FName("State.Dead"));
	ASC->RegisterGameplayTagEvent(DeathTag, EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &UMSMonsterSpawnSubsystem::OnEnemyDeathTagChanged, Enemy);
}

void UMSMonsterSpawnSubsystem::OnEnemyDeathTagChanged(const FGameplayTag Tag, int32 NewCount, AMSBaseEnemy* Enemy)
{
	if (NewCount > 0)  // 태그 추가됨 = 사망
	{
		HandleEnemyDeath(Enemy);
	}
}

void UMSMonsterSpawnSubsystem::HandleEnemyDeath(AMSBaseEnemy* Enemy)
{
	if (!Enemy)
	{
		return;
	}

	// 어느 풀에 속한 적인지 찾기
	FMSEnemyPool* OwningPool = nullptr;
	if (NormalEnemyPool.ActiveEnemies.Contains(Enemy))
	{
		OwningPool = &NormalEnemyPool;
	}
	else if (EliteEnemyPool.ActiveEnemies.Contains(Enemy))
	{
		OwningPool = &EliteEnemyPool;
	}
	else if (BossEnemyPool.ActiveEnemies.Contains(Enemy))
	{
		OwningPool = &BossEnemyPool;
	}

	if (!OwningPool)
	{
		UE_LOG(LogTemp, Warning, TEXT("Dead enemy not found in any active pool!"));
		return;
	}

	// 액티브 풀에서 제거
	OwningPool->ActiveEnemies.Remove(Enemy);
	CurrentActiveCount--;
	TotalKilledCount++;

	// 이벤트 브로드캐스트
	OnMonsterDied.Broadcast(Enemy, TotalKilledCount);

	// 2초 후 풀로 반환 (사망 애니메이션 재생 시간)
	FTimerHandle DeathDelayTimer;
	GetWorld()->GetTimerManager().SetTimer(
		DeathDelayTimer,
		FTimerDelegate::CreateUObject(this, &UMSMonsterSpawnSubsystem::ReturnEnemyToPool, Enemy, OwningPool),
		2.0f,
		false
	);
}

void UMSMonsterSpawnSubsystem::ReturnEnemyToPool(AMSBaseEnemy* Enemy, FMSEnemyPool* Pool)
{
	if (!Enemy || !Pool)
	{
		return;
	}

	DeactivateEnemy(Enemy);
	Pool->FreeEnemies.Add(Enemy);

	UE_LOG(LogTemp, Verbose, TEXT("Enemy returned to pool. Free: %d, Active: %d"),
		Pool->FreeEnemies.Num(), Pool->ActiveEnemies.Num());
}

void UMSMonsterSpawnSubsystem::UpdateDifficultyWave()
{
	int32 NewWave = (TotalSpawnedCount / MonstersPerWave) + 1;

	if (NewWave != CurrentWave)
	{
		CurrentWave = NewWave;
		CurrentDifficultyMultiplier = 1.0f + (CurrentWave - 1) * DifficultyScalePerWave;
		CurrentDifficultyMultiplier = FMath::Min(CurrentDifficultyMultiplier, MaxDifficultyMultiplier);

		UE_LOG(LogTemp, Log, TEXT("Wave %d reached! Difficulty multiplier: %.2f"),
			CurrentWave, CurrentDifficultyMultiplier);

		OnWaveChanged.Broadcast(CurrentWave, CurrentDifficultyMultiplier);
	}
}

// ========================================================================================
// 설정 함수
// ========================================================================================

void UMSMonsterSpawnSubsystem::SetSpawnInterval(float NewInterval)
{
	SpawnInterval = FMath::Max(0.1f, NewInterval);

	// 스폰 중이면 타이머 재시작
	if (bIsSpawning)
	{
		StopSpawning();
		StartSpawning();
	}
}

void UMSMonsterSpawnSubsystem::SetMaxActiveMonsters(int32 NewMax)
{
	MaxActiveMonsters = FMath::Max(1, NewMax);
}

void UMSMonsterSpawnSubsystem::SetSpawnRadius(float NewRadius)
{
	SpawnRadius = FMath::Max(500.0f, NewRadius);
}

void UMSMonsterSpawnSubsystem::SetMonsterDataTable(UDataTable* NewDataTable)
{
	if (!NewDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("SetMonsterDataTable: NewDataTable is null!"));
		return;
	}

	// 기존 데이터 정리
	CachedMonsterData.Empty();
	MonsterPoolMap.Empty();

	// 새 DataTable 할당
	MonsterStaticDataTable = NewDataTable;

	// 데이터 다시 로드
	LoadMonsterDataTable();

	// 풀 재생성
	PrewarmPools();

	UE_LOG(LogTemp, Log, TEXT("MonsterDataTable set and reloaded successfully"));
}

void UMSMonsterSpawnSubsystem::SetDifficultyMultiplier(float NewMultiplier)
{
	CurrentDifficultyMultiplier = FMath::Clamp(NewMultiplier, 1.0f, MaxDifficultyMultiplier);
	OnWaveChanged.Broadcast(CurrentWave, CurrentDifficultyMultiplier);
}

void UMSMonsterSpawnSubsystem::ResetDifficulty()
{
	CurrentWave = 1;
	CurrentDifficultyMultiplier = 1.0f;
	TotalSpawnedCount = 0;
	TotalKilledCount = 0;

	UE_LOG(LogTemp, Log, TEXT("Difficulty reset to Wave 1"));
}

UMSMonsterSpawnSubsystem* UMSMonsterSpawnSubsystem::GetMonsterSpawnSubsystem(UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	if (UWorld* World = WorldContextObject->GetWorld())
	{
		return World->GetSubsystem<UMSMonsterSpawnSubsystem>();
	}

	return nullptr;
}
