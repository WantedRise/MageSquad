// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/DataTable.h"
#include "NavigationSystem.h"
#include "GameplayTagContainer.h"
#include "MSMonsterSpawnSubsystem.generated.h"

// Forward declarations
class AMSBaseEnemy;
class UDataTable;
struct FMSMonsterStaticData;

/**
 * 몬스터 풀 구조체
 */
USTRUCT()
struct FMSEnemyPool
{
	GENERATED_BODY()

	UPROPERTY()
	TSubclassOf<AMSBaseEnemy> EnemyClass;

	UPROPERTY()
	TArray<TObjectPtr<AMSBaseEnemy>> FreeEnemies;

	UPROPERTY()
	TArray<TObjectPtr<AMSBaseEnemy>> ActiveEnemies;

	int32 InitialPoolSize = 10;
};

/**
 * 델리게이트 선언
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMonsterSpawnedSignature,
	AMSBaseEnemy*, SpawnedMonster, FName, MonsterID, int32, CurrentWave);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMonsterDiedSignature,
	AMSBaseEnemy*, DeadMonster, int32, TotalKillCount);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWaveChangedSignature,
	int32, NewWave, float, DifficultyMultiplier);

/**
 * 작성자 : 임희섭
 * 작성일 : 2025/12/12
 * 몬스터 스폰 관리 및 웨이브 시스템 담당
 * 연속 스폰, 오브젝트 풀링, 난이도 스케일링 지원
 */
UCLASS()
class MAGESQUAD_API UMSMonsterSpawnSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// 서브시스템 라이프사이클
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// 스폰 제어
	UFUNCTION(BlueprintCallable, Category = "Monster Spawn")
	void StartSpawning();

	UFUNCTION(BlueprintCallable, Category = "Monster Spawn")
	void StopSpawning();

	UFUNCTION(BlueprintCallable, Category = "Monster Spawn")
	void ClearAllMonsters();

	// 설정
	UFUNCTION(BlueprintCallable, Category = "Monster Spawn")
	void SetSpawnInterval(float NewInterval);

	UFUNCTION(BlueprintCallable, Category = "Monster Spawn")
	void SetMaxActiveMonsters(int32 NewMax);

	UFUNCTION(BlueprintCallable, Category = "Monster Spawn")
	void SetSpawnRadius(float NewRadius);

	// DataTable 설정
	UFUNCTION(BlueprintCallable, Category = "Monster Spawn")
	void SetMonsterDataTable(UDataTable* NewDataTable);

	// 난이도 조정
	UFUNCTION(BlueprintCallable, Category = "Monster Spawn")
	void SetDifficultyMultiplier(float NewMultiplier);

	UFUNCTION(BlueprintCallable, Category = "Monster Spawn")
	void ResetDifficulty();

	// Getter
	UFUNCTION(BlueprintPure, Category = "Monster Spawn")
	int32 GetCurrentActiveCount() const { return CurrentActiveCount; }

	UFUNCTION(BlueprintPure, Category = "Monster Spawn")
	int32 GetTotalSpawnedCount() const { return TotalSpawnedCount; }

	UFUNCTION(BlueprintPure, Category = "Monster Spawn")
	int32 GetTotalKilledCount() const { return TotalKilledCount; }

	UFUNCTION(BlueprintPure, Category = "Monster Spawn")
	int32 GetCurrentWave() const { return CurrentWave; }

	UFUNCTION(BlueprintPure, Category = "Monster Spawn")
	float GetCurrentDifficultyMultiplier() const { return CurrentDifficultyMultiplier; }

	UFUNCTION(BlueprintPure, Category = "Monster Spawn")
	bool IsSpawning() const { return bIsSpawning; }

	// 정적 접근자
	UFUNCTION(BlueprintCallable, Category = "Monster Spawn", meta = (WorldContext = "WorldContextObject"))
	static UMSMonsterSpawnSubsystem* GetMonsterSpawnSubsystem(UObject* WorldContextObject);

	// 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Monster Spawn Events")
	FOnMonsterSpawnedSignature OnMonsterSpawned;

	UPROPERTY(BlueprintAssignable, Category = "Monster Spawn Events")
	FOnMonsterDiedSignature OnMonsterDied;

	UPROPERTY(BlueprintAssignable, Category = "Monster Spawn Events")
	FOnWaveChangedSignature OnWaveChanged;

private:
	// 초기화
	void LoadMonsterDataTable();
	void AssignMonsterToPool(const FName& RowName);
	void PrewarmPools();
	void PrewarmPool(FMSEnemyPool& Pool);

	// 스폰 로직
	void SpawnMonsterTick();
	AMSBaseEnemy* SpawnMonster(const FName& MonsterID);
	bool GetRandomSpawnLocation(FVector& OutLocation);

	// 몬스터 초기화
	void InitializeMonsterFromData(AMSBaseEnemy* Enemy, const FName& MonsterID);
	void ActivateEnemy(AMSBaseEnemy* Enemy, const FVector& Location);
	void DeactivateEnemy(AMSBaseEnemy* Enemy);

	// 풀 반환
	void BindEnemyDeathEvent(AMSBaseEnemy* Enemy);
	void OnEnemyDeathTagChanged(const FGameplayTag Tag, int32 NewCount, AMSBaseEnemy* Enemy);
	void HandleEnemyDeath(AMSBaseEnemy* Enemy);
	void ReturnEnemyToPool(AMSBaseEnemy* Enemy, FMSEnemyPool* Pool);

	// 난이도
	void UpdateDifficultyWave();

private:
	// DataTable 참조
	UPROPERTY(EditDefaultsOnly, Category = "Monster Data")
	TObjectPtr<UDataTable> MonsterStaticDataTable;

	// 풀 관리
	FMSEnemyPool NormalEnemyPool;
	FMSEnemyPool EliteEnemyPool;
	FMSEnemyPool BossEnemyPool;
	TMap<FName, FMSEnemyPool*> MonsterPoolMap;
	TMap<FName, FMSMonsterStaticData*> CachedMonsterData;

	// 풀 크기 설정
	UPROPERTY(EditDefaultsOnly, Category = "Pool Config")
	int32 NormalEnemyPoolSize = 30;

	UPROPERTY(EditDefaultsOnly, Category = "Pool Config")
	int32 EliteEnemyPoolSize = 10;

	UPROPERTY(EditDefaultsOnly, Category = "Pool Config")
	int32 BossEnemyPoolSize = 3;

	// 스폰 설정
	UPROPERTY(EditDefaultsOnly, Category = "Spawn Config")
	float SpawnInterval = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Spawn Config")
	int32 MaxActiveMonsters = 50;

	UPROPERTY(EditDefaultsOnly, Category = "Spawn Config")
	float SpawnRadius = 2000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Spawn Config")
	float MinSpawnDistance = 800.0f;

	// 난이도 스케일링
	UPROPERTY(EditDefaultsOnly, Category = "Difficulty Scaling")
	float DifficultyScalePerWave = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "Difficulty Scaling")
	float MaxDifficultyMultiplier = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Difficulty Scaling")
	int32 MonstersPerWave = 10;

	// 런타임 상태
	FTimerHandle SpawnTimerHandle;
	bool bIsSpawning = false;
	int32 CurrentActiveCount = 0;
	int32 TotalSpawnedCount = 0;
	int32 TotalKilledCount = 0;
	int32 CurrentWave = 1;
	float CurrentDifficultyMultiplier = 1.0f;

	// 네비게이션
	UPROPERTY()
	TObjectPtr<class UNavigationSystemV1> NavSystem;
};
