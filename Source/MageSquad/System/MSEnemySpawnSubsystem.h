// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "MSEnemySpawnSubsystem.generated.h"

/*
 * 작성자 : 임희섭
 * 작성일 : 2025/12/15
 * 리팩토링 : 2025/12/15
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
 * - NavMesh 기반 스폰 위치 검증
 * - GAS 기반 몬스터 초기화
 */

// Forward declarations
class AMSBaseEnemy;
class UNavigationSystemV1;
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
	TObjectPtr<class UDA_EnemyAnimationSet> AnimationSet;

	// 스탯 정보
	float MaxHealth = 100.0f;
	float MoveSpeed = 400.0f;
	float AttackDamage = 10.0f;
	float AttackRange = 200.0f;
	bool bIsRanged = false;

	// GAS 관련
	TArray<TSubclassOf<UGameplayAbility>> StartAbilities;
	TArray<TSubclassOf<UGameplayEffect>> StartEffects;
};

/**
 * 몬스터 스폰 서브시스템
 */
UCLASS()
class MAGESQUAD_API UMSEnemySpawnSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	//~=============================================================================
	// Subsystem Lifecycle
	//~=============================================================================
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	void InitializePool();

	//~=============================================================================
	// Spawn Control (Blueprint Callable)
	//~=============================================================================
	
	/** 자동 스폰 시작 (서버에서만 동작) */
	UFUNCTION(BlueprintCallable, Category = "Monster Spawn")
	void StartSpawning();

	/** 자동 스폰 중지 */
	UFUNCTION(BlueprintCallable, Category = "Monster Spawn")
	void StopSpawning();

	/** 모든 활성 몬스터 제거 및 풀 초기화 */
	UFUNCTION(BlueprintCallable, Category = "Monster Spawn")
	void ClearAllMonsters();

	/** 특정 타입의 몬스터 수동 스폰 (서버에서만 동작) */
	UFUNCTION(BlueprintCallable, Category = "Monster Spawn")
	AMSBaseEnemy* SpawnMonsterByID(const FName& MonsterID, const FVector& Location);
	
	
	UFUNCTION(BlueprintCallable, Category = "Monster Spawn|Config")
	void SetSpawnInterval(float NewInterval);

	UFUNCTION(BlueprintCallable, Category = "Monster Spawn|Config")
	void SetMaxActiveMonsters(int32 NewMax);

	UFUNCTION(BlueprintCallable, Category = "Monster Spawn|Config")
	void SetSpawnRadius(float NewRadius);
	
	UFUNCTION(BlueprintCallable, Category = "Monster Spawn|Config")
	void SetSpawnCountPerTick(int InSpawnCountPerTick);

	UFUNCTION(BlueprintCallable, Category = "Monster Spawn|Config")
	void SetMonsterDataTable(UDataTable* NewDataTable);
	
	
	UFUNCTION(BlueprintPure, Category = "Monster Spawn|Info")
	int32 GetCurrentActiveCount() const { return CurrentActiveCount; }

	UFUNCTION(BlueprintPure, Category = "Monster Spawn|Info")
	int32 GetTotalSpawnedCount() const { return TotalSpawnedCount; }

	UFUNCTION(BlueprintPure, Category = "Monster Spawn|Info")
	bool IsSpawning() const { return bIsSpawning; }
	
	
	UFUNCTION(BlueprintCallable, Category = "Monster Spawn", meta = (WorldContext = "WorldContextObject"))
	static UMSEnemySpawnSubsystem* Get(UObject* WorldContextObject);
	
	
	/** 외부에서 Enemy 사망 시 호출하여 풀로 반환 */
	UFUNCTION(BlueprintCallable, Category = "Monster Spawn|Pool")
	void ReturnEnemyToPool(AMSBaseEnemy* Enemy);

private:
	/** DataTable에서 몬스터 데이터 로드 및 에셋 사전 로드 */
	void LoadMonsterDataTable();
	
	/** 몬스터 ID를 분석하여 적절한 풀에 매핑 */
	void AssignMonsterToPool(const FName& RowName);
	
	/** 모든 풀 사전 생성 */
	void PrewarmPools();
	
	/** 특정 풀 사전 생성 */
	void PrewarmPool(FMSEnemyPool& Pool);
	
	/** 타이머 콜백: 주기적으로 랜덤 몬스터 스폰 */
	void SpawnMonsterTick();
	
	/** 내부 스폰 로직 (풀에서 가져오거나 새로 생성) */
	AMSBaseEnemy* SpawnMonsterInternal(const FName& MonsterID, const FVector& Location);
	
	/** NavMesh 기반 랜덤 스폰 위치 검색 */
	bool GetRandomSpawnLocation(FVector& OutLocation);

	/** 해당 위치가 플레이어 뷰포트에 보이는지 체크 */
	bool IsLocationVisibleToPlayer(APlayerController* PC, const FVector& Location);
	
	/** 멀티 고려 - 해당 위치가 플레이어 뷰포트에 보이는지 체크 */
	bool IsLocationVisibleToAnyPlayer(const FVector& Location);
	
	/** 화면 가장자리의 랜덤한 지점 반환 (화면 밖)*/
	FVector2D GetRandomScreenEdgePoint(int32 ViewportSizeX, int32 ViewportSizeY, float Margin);

public:
	/** DataTable 데이터로 Enemy 초기화 (메시, 애니메이션, GAS) */
	void InitializeEnemyFromData(AMSBaseEnemy* Enemy, const FName& MonsterID);
	/** Enemy 활성화 (위치 설정, AI 시작) */
	void ActivateEnemy(AMSBaseEnemy* Enemy, const FVector& Location) const;
	
private:
	/** Enemy 비활성화 (숨김, 콜리전 끄기, AI 정지) */
	void DeactivateEnemy(AMSBaseEnemy* Enemy);
	
	/** GAS 상태 완전 초기화 (태그, 이펙트, 어빌리티 제거) */
	void ResetEnemyGASState(AMSBaseEnemy* Enemy);
	
	/** Enemy의 사망 태그 이벤트 바인딩 */
	void BindEnemyDeathEvent(AMSBaseEnemy* Enemy);
	
	/** Enemy의 사망 태그 이벤트 언바인딩 */
	void UnbindEnemyDeathEvent(AMSBaseEnemy* Enemy);
	
	/** 사망 태그 변경 콜백 */
	UFUNCTION()
	void OnEnemyDeathTagChanged(const FGameplayTag Tag, int32 NewCount, AMSBaseEnemy* Enemy);
	
	/** 사망 처리 및 풀 반환 예약 */
	void HandleEnemyDeath(AMSBaseEnemy* Enemy);
	
	/** 타이머 콜백: 사망 애니메이션 후 풀로 반환 */
	void ReturnEnemyToPoolInternal(AMSBaseEnemy* Enemy, FMSEnemyPool* Pool);
	
	/** Enemy가 속한 풀 찾기 (O(1) 조회) */
	FMSEnemyPool* FindPoolForEnemy(AMSBaseEnemy* Enemy) const;
	
	/** 서버 권한 체크 */
	bool HasAuthority() const;

private:
	
	/** 몬스터 정적 데이터 테이블 */
	UPROPERTY(EditDefaultsOnly, Category = "Monster Data")
	TObjectPtr<UDataTable> MonsterStaticDataTable;

	/** 몬스터별 캐시된 데이터 (에셋 사전 로드 포함) */
	UPROPERTY()
	TMap<FName, FMSCachedEnemyData> CachedMonsterData;
	
	/*Normal 몬스터 데이터 캐싱용 맵*/
	UPROPERTY()
	TMap<FName, FMSCachedEnemyData> CachedNormalMonsterData;
	
	/*Elite 몬스터 데이터 캐싱용 맵*/
	UPROPERTY()
	TMap<FName, FMSCachedEnemyData> CachedEliteMonsterData;
	
	/*Boss몬스터 데이터 캐싱용 맵*/
	UPROPERTY()
	TMap<FName, FMSCachedEnemyData> CachedBossMonsterData;
	
	UPROPERTY()
	FMSEnemyPool NormalEnemyPool;

	UPROPERTY()
	FMSEnemyPool EliteEnemyPool;

	UPROPERTY()
	FMSEnemyPool BossEnemyPool;

	/** MonsterID -> Pool 매핑 */
	TMap<FName, FMSEnemyPool*> MonsterPoolMap;

	/** Enemy -> Pool 역참조 (O(1) 풀 검색) */
	TMap<TObjectPtr<AMSBaseEnemy>, FMSEnemyPool*> EnemyToPoolMap;
	
	const int32 NormalEnemyPoolSize = 10;

	const int32 EliteEnemyPoolSize = 5;

	const int32 BossEnemyPoolSize = 1;
	
	/** 스폰 간격 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Spawn Config")
	float SpawnInterval = 3.0f;

	/** 최대 동시 활성 몬스터 수 */
	UPROPERTY(EditDefaultsOnly, Category = "Spawn Config")
	int32 MaxActiveMonsters = 50;

	/** 플레이어 주변 스폰 반경 */
	UPROPERTY(EditDefaultsOnly, Category = "Spawn Config")
	float SpawnRadius = 2000.0f;

	/** 플레이어로부터 최소 스폰 거리 */
	UPROPERTY(EditDefaultsOnly, Category = "Spawn Config")
	float MinSpawnDistance = 800.0f;

	/** 사망 후 풀 반환 대기 시간 (사망 애니메이션 재생 시간) */
	UPROPERTY(EditDefaultsOnly, Category = "Spawn Config")
	float DeathAnimationDuration = 2.0f;
	
	/* 스폰마다 몇 마리씩 스폰할 건지 정하는 변수*/
	UPROPERTY(EditDefaultsOnly, Category = "Spawn Config")
	int SpawnCountPerTick = 1;

	//~=============================================================================
	// Runtime State
	//~=============================================================================
	
	/** 스폰 타이머 핸들 */
	FTimerHandle SpawnTimerHandle;

	/** 스폰 중 여부 */
	bool bIsSpawning = false;

	/** 현재 활성 몬스터 수 */
	int32 CurrentActiveCount = 0;

	/** 총 스폰된 몬스터 수 (누적) */
	int32 TotalSpawnedCount = 0;

	//~=============================================================================
	// Navigation
	//~=============================================================================
	
	UPROPERTY()
	TObjectPtr<UNavigationSystemV1> NavSystem;
};