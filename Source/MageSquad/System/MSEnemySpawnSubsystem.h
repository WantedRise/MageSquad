// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "MSEnemySpawnSubsystem.generated.h"

/*
 * 작성자 : 임희섭
 * 작성일 : 2025/12/15
 * 
 * 몬스터 스폰 관리 전용 서브시스템
 * - 오브젝트 풀링 기반 효율적인 스폰 관리
 * - 멀티플레이어 리슨 서버 방식 지원 (서버 권한 체크)
 * - 메모리 안전성 강화 (델리게이트 관리, GAS 상태 초기화)
 * - 에셋 사전 로딩으로 프레임 드롭 방지
 * 
 * 주요 기능:
 * - 타입별 오브젝트 풀 (Normal/Elite/Boss)
 * - 자동 스폰 시스템 (간격/최대 수 제어)
 * - TileMap 기반 스폰 위치 검증
 * - GAS 기반 몬스터 초기화
 */

// Forward declarations
class AMSBaseEnemy;
class UNavigationSystemV1;
class AMSSpawnTileMap;
struct FMSEnemyStaticData;

/**
 * 몬스터 풀 구조체
 * 타입별로 Free/Active 상태 관리
 */
USTRUCT()
struct FMSEnemyPool
{
	GENERATED_BODY()

	// 풀에서 사용할 Enemy 클래스
	UPROPERTY()
	TSubclassOf<AMSBaseEnemy> EnemyClass;

	// 비활성 상태 (풀에서 대기 중)
	UPROPERTY()
	TArray<TObjectPtr<AMSBaseEnemy>> FreeEnemies;

	// 활성 상태 (게임 월드에 스폰됨)
	UPROPERTY()
	TArray<TObjectPtr<AMSBaseEnemy>> ActiveEnemies;

	// 초기 풀 크기
	int32 InitialPoolSize = 10;
};

/**
 * 몬스터별 캐시된 데이터
 * DataTable 포인터 대신 값 복사로 안전성 확보
 */
USTRUCT()
struct FMSCachedEnemyData
{
	GENERATED_BODY()

	// 사전 로드된 에셋들
	UPROPERTY()
	TObjectPtr<USkeletalMesh> SkeletalMesh;
	UPROPERTY()
	TObjectPtr<USkeletalMesh> Phase2SkeletalMesh;

	UPROPERTY()
	TObjectPtr<class UDA_EnemyAnimationSet> AnimationSet;

	// 스탯 정보
	float MaxHealth = 100.0f;
	float MoveSpeed = 400.0f;
	float AttackDamage = 10.0f;
	float AttackRange = 200.0f;
	bool bIsRanged = false;
	float DropExpValue = 10.f;
	
	// 투사체 관련
	TSubclassOf<class UProjectileStaticData> ProjectileDataClass;
	
	// GAS 관련
	UPROPERTY()
	TObjectPtr<class UDA_EnemyAbilityData> EnemyAbilities;
	
	// Sound 관련
	UPROPERTY()
	TObjectPtr<class UDA_EnemyStaticSoundData> EnemySounds;
	
	// 보스 인디케이터를 위한 이미지 추가 - 김준형
	UPROPERTY()
	TObjectPtr<class UTexture2D> IndicatorImage;
};

// 스폰 요청을 큐로 관리하기 위해 제작하는 요청 큐
USTRUCT()
struct FMSPendingSpawnRequest
{
	GENERATED_BODY()

	FName MonsterID;
	FVector Location;
    
	FMSPendingSpawnRequest() = default;
	FMSPendingSpawnRequest(const FName& InID, const FVector& InLoc)
		: MonsterID(InID), Location(InLoc) {}
};

UCLASS()
class MAGESQUAD_API UMSEnemySpawnSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	void InitializePool();
	
	UFUNCTION(BlueprintCallable, Category = "Monster Spawn")
	void StartSpawning();

	UFUNCTION(BlueprintCallable, Category = "Monster Spawn")
	void StopSpawning();

	UFUNCTION(BlueprintCallable, Category = "Monster Spawn")
	void ClearAllMonsters();

	UFUNCTION(BlueprintCallable, Category = "Monster Spawn")
	AMSBaseEnemy* SpawnMonsterByID(const FName& MonsterID, const FVector& Location);
	
	UFUNCTION(BlueprintCallable, Category = "Monster Spawn|Config")
	void SetSpawnInterval(float NewInterval);
	
	UFUNCTION(BlueprintCallable, Category = "Monster Spawn|Config")
	void SetEliteSpawnInterval(float NewInterval);

	UFUNCTION(BlueprintCallable, Category = "Monster Spawn|Config")
	void SetMaxActiveMonsters(int32 NewMax);
	
	UFUNCTION(BlueprintCallable, Category = "Monster Spawn|Config")
	void SetSpawnCountPerTick(int InSpawnCountPerTick);
	
	UFUNCTION(BlueprintPure, Category = "Monster Spawn|Info")
	int32 GetCurrentActiveCount() const { return CurrentActiveCount; }

	UFUNCTION(BlueprintPure, Category = "Monster Spawn|Info")
	int32 GetTotalSpawnedCount() const { return TotalSpawnedCount; }

	UFUNCTION(BlueprintPure, Category = "Monster Spawn|Info")
	bool IsSpawning() const { return bIsSpawning; }
	
	void InitializeEnemyFromData(AMSBaseEnemy* Enemy, const FName& MonsterID);

	void ActivateEnemy(AMSBaseEnemy* Enemy, const FVector& Location = FVector()) const;

	// 타일맵 찾기 및 캐싱
	AMSSpawnTileMap* GetSpawnTileMap();
	
	UFUNCTION(BlueprintCallable, Category = "Monster Spawn", meta = (WorldContext = "WorldContextObject"))
	static UMSEnemySpawnSubsystem* Get(UObject* WorldContextObject);

private:
	void LoadMonsterDataTable();
	
	// 몬스터 ID를 분석하여 적절한 풀에 매핑
	void AssignMonsterToPool(const FName& RowName);
	
	void PrewarmPools();
	
	void PrewarmPool(FMSEnemyPool& Pool);
	
	void SpawnMonsterTick();
	
	void SpawnEliteMonsterTick();
	
	// 내부 스폰 로직 (풀에서 가져오거나 새로 생성)
	AMSBaseEnemy* SpawnMonsterInternal(const FName& MonsterID, const FVector& Location);
	
	// 캐싱된 타일 목록에서 랜덤 위치 선택
	bool GetRandomSpawnLocationFromTiles(
		const TArray<struct FMSSpawnTile>& InvisibleTiles,
		const FVector& PlayerLocation,
		FVector& OutLocation);
	
	// 스폰 요청을 큐에 추가
	void QueueSpawnRequest(const FName& MonsterID, const FVector& Location);
    
	// 큐 처리 (타이머 콜백)
	void ProcessSpawnQueue();
	
	void DeactivateEnemy(AMSBaseEnemy* Enemy);
	
	void ResetEnemyGASState(AMSBaseEnemy* Enemy);
	
	void BindEnemyDeathEvent(AMSBaseEnemy* Enemy);
	
	void UnbindEnemyDeathEvent(AMSBaseEnemy* Enemy);
	
	UFUNCTION()
	void OnEnemyDeathTagChanged(const FGameplayTag Tag, int32 NewCount, AMSBaseEnemy* Enemy);
	
	void HandleEnemyDeath(AMSBaseEnemy* Enemy);
	
	void ReturnEnemyToPoolInternal(AMSBaseEnemy* Enemy, FMSEnemyPool* Pool);
	
	FMSEnemyPool* FindPoolForEnemy(AMSBaseEnemy* Enemy) const;
	
	bool HasAuthority() const;

	// 모든 플레이어 컨트롤러 수집
	TArray<APlayerController*> GetAllPlayerControllers() const;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Monster Data")
	TObjectPtr<UDataTable> MonsterStaticDataTable;

	UPROPERTY()
	TMap<FName, FMSCachedEnemyData> CachedMonsterData;
	
	// Normal 몬스터 데이터 캐싱용 맵
	UPROPERTY()
	TMap<FName, FMSCachedEnemyData> CachedNormalMonsterData;
	
	// Elite 몬스터 데이터 캐싱용 맵
	UPROPERTY()
	TMap<FName, FMSCachedEnemyData> CachedEliteMonsterData;
	
	// Boss몬스터 데이터 캐싱용 맵
	UPROPERTY()
	TMap<FName, FMSCachedEnemyData> CachedBossMonsterData;
	
	TArray<FName> CachedNormalMonsterKeys;
	
	// 대기 중인 스폰 요청
	TArray<FMSPendingSpawnRequest> PendingSpawnQueue;
    
	// 큐 처리 타이머 
	FTimerHandle SpawnQueueTimerHandle;
    
	// 프레임당 최대 스폰 수
	int32 MaxSpawnsPerFrame = 3;
	
	UPROPERTY()
	FMSEnemyPool NormalEnemyPool;

	UPROPERTY()
	FMSEnemyPool EliteEnemyPool;

	UPROPERTY()
	FMSEnemyPool BossEnemyPool;

	// MonsterID -> Pool 매핑
	TMap<FName, FMSEnemyPool*> MonsterPoolMap;

	// Enemy -> Pool 역참조 (O(1) 풀 검색)
	TMap<TObjectPtr<AMSBaseEnemy>, FMSEnemyPool*> EnemyToPoolMap;
	
	const int32 NormalEnemyPoolSize = 100;

	const int32 EliteEnemyPoolSize = 5;

	const int32 BossEnemyPoolSize = 1;
	
	// 초기화 체크
	bool bShouldSkipInitialization = false;
	
	UPROPERTY(EditDefaultsOnly, Category = "Spawn Config")
	float SpawnInterval = 3.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Spawn Config")
	float EliteSpawnInterval = 60.0f;

	// 최대 동시 활성 몬스터 수
	UPROPERTY(EditDefaultsOnly, Category = "Spawn Config")
	int32 MaxActiveMonsters = 50;
	
	// 스폰마다 몇 마리씩 스폰할 건지 정하는 변수
	UPROPERTY(EditDefaultsOnly, Category = "Spawn Config")
	int SpawnCountPerTick = 1;
	
	// 스폰 타이머 핸들
	FTimerHandle SpawnTimerHandle;
	FTimerHandle EliteSpawnTimerHandle;

	// 스폰 중 여부 
	bool bIsSpawning = false;

	// 현재 활성 몬스터 수
	int32 CurrentActiveCount = 0;

	// 총 스폰된 몬스터 수 (누적)
	int32 TotalSpawnedCount = 0;
	
	uint32 PlayerCount = 1;
	
	UPROPERTY()
	TObjectPtr<UNavigationSystemV1> NavSystem;
	
	// 타일맵 레퍼런스
	UPROPERTY()
	TWeakObjectPtr<AMSSpawnTileMap> SpawnTileMap;
};