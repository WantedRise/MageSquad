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
#include "MageSquad.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Animation/Enemy/MSEnemyAnimInstance.h"
#include "Enemy/AIController/MSBaseAIController.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

//~=============================================================================
// Subsystem Lifecycle
//~=============================================================================

void UMSEnemySpawnSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (!GetWorld()->GetName().Contains(TEXT("Lvl_Dev_Lim")))
	{
		return;
	}

	if (GetWorld()->WorldType != EWorldType::PIE && GetWorld()->WorldType != EWorldType::Game)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SpawnSystem] Not PIE/Game world, skipping initialization"));
		return;
	}

	// if (!HasAuthority())
	// {
	// 	return;
	// }

	// NavSystem 참조 획득
	NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

	// DataTable 로드 및 에셋 사전 로딩
	LoadMonsterDataTable();

	// 풀 사전 생성
	PrewarmPools();

	UE_LOG(LogTemp, Log, TEXT("[MonsterSpawn] Subsystem Initialized - Server: %s"),
	       HasAuthority() ? TEXT("YES") : TEXT("NO"));
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
	MonsterPoolMap.Empty();
	EnemyToPoolMap.Empty();

	UE_LOG(LogTemp, Log, TEXT("[MonsterSpawn] Subsystem Deinitialized"));

	Super::Deinitialize();
}

//~=============================================================================
// Initialization
//~=============================================================================

void UMSEnemySpawnSubsystem::LoadMonsterDataTable()
{
	// if (!MonsterStaticDataTable)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("[MonsterSpawn] MonsterStaticDataTable is not assigned!"));
	// 	return;
	// }
	if (!MonsterStaticDataTable)
	{
		MonsterStaticDataTable = LoadObject<UDataTable>(nullptr,
		                                                TEXT("/Game/Data/Enemy/DT/DT_MonsterStaticData"));
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

		// GAS 데이터 복사
		CachedData.StartAbilities = RowData->StartAbilities;
		CachedData.StartEffects = RowData->StartEffects;

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
	}
	else if (RowString.StartsWith(TEXT("Elite_")))
	{
		MonsterPoolMap.Add(RowName, &EliteEnemyPool);
	}
	else if (RowString.StartsWith(TEXT("Boss_")))
	{
		MonsterPoolMap.Add(RowName, &BossEnemyPool);
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
		//Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Params.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
		// 맵 밖으로 스폰 (비활성 상태)
		AMSBaseEnemy* Enemy = World->SpawnActor<AMSBaseEnemy>(
			Pool.EnemyClass,
			FVector(0, 0, 100.0f),
			FRotator::ZeroRotator,
			Params
		);

		if (Enemy)
		{   
			// ✅ 풀링 모드 설정 (AI Controller 생성 방지)
			if (AMSNormalEnemy* NormalEnemy = Cast<AMSNormalEnemy>(Enemy))
			{
				NormalEnemy->SetPoolingMode(true);
			}
			
			Enemy->SetNetDormancy(DORM_Initial);  // 완전 휴면
			DeactivateEnemy(Enemy);
			Pool.FreeEnemies.Add(Enemy);
			
			// UE_LOG(LogTemp, Log, TEXT("Pooled: %s | LocalRole: %d | RemoteRole: %d"),
			// 	*Enemy->GetName(),
			// 	(int32)Enemy->GetLocalRole(),
			// 	(int32)Enemy->GetRemoteRole()
			// );
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[MonsterSpawn] Prewarmed pool: %s (%d enemies)"),
	       *Pool.EnemyClass->GetName(), Pool.InitialPoolSize);
}

//~=============================================================================
// Spawn Control
//~=============================================================================

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

//~=============================================================================
// Spawn Logic
//~=============================================================================

void UMSEnemySpawnSubsystem::SpawnMonsterTick()
{
	// 서버 권한 체크 (타이머가 클라이언트에서 실행될 수 없지만 안전장치)
	if (!HasAuthority())
	{
		return;
	}

	// 최대 수량 체크
	if (CurrentActiveCount >= MaxActiveMonsters)
	{
		return;
	}

	// 랜덤 몬스터 타입 선택
	TArray<FName> AllMonsterTypes;
	CachedMonsterData.GetKeys(AllMonsterTypes);

	if (AllMonsterTypes.Num() == 0)
	{
		return;
	}

	const FName SelectedMonsterID = AllMonsterTypes[FMath::RandRange(0, AllMonsterTypes.Num() - 1)];

	// 랜덤 위치 검색
	FVector SpawnLocation;
	if (!GetRandomSpawnLocation(SpawnLocation))
	{
		return;
	}

	// 스폰
	SpawnMonsterInternal(SelectedMonsterID, SpawnLocation);
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

	// 활성화
	ActivateEnemy(Enemy, Location);
	
	// 몬스터 ID 설정
	Enemy->SetMonsterID(MonsterID);

	// DataTable 데이터로 초기화
	InitializeEnemyFromData(Enemy, MonsterID);
	
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

bool UMSEnemySpawnSubsystem::GetRandomSpawnLocation(FVector& OutLocation)
{
	if (!NavSystem)
	{
		NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
		if (!NavSystem)
		{
			return false;
		}
	}

	// 플레이어 위치 기준
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MonsterSpawn] No player pawn found"));
		return false;
	}

	const FVector PlayerLocation = PlayerPawn->GetActorLocation();
	const int32 MaxAttempts = 10;

	for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
	{
		// 랜덤 각도
		const float RandomAngle = FMath::FRandRange(0.0f, 360.0f);
		const float RandomDistance = FMath::FRandRange(MinSpawnDistance, SpawnRadius);

		// 원형 분포
		const FVector Offset = FVector(
			FMath::Cos(FMath::DegreesToRadians(RandomAngle)) * RandomDistance,
			FMath::Sin(FMath::DegreesToRadians(RandomAngle)) * RandomDistance,
			0.0f
		);

		const FVector CandidateLocation = PlayerLocation + Offset;

		// NavMesh 검증
		FNavLocation NavLoc;
		if (NavSystem->ProjectPointToNavigation(CandidateLocation, NavLoc, FVector(500.0f, 500.0f, 1000.0f)))
		{
			OutLocation = NavLoc.Location;
			OutLocation.Z = 92.f;
			return true;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[MonsterSpawn] Failed to find valid spawn location after %d attempts"), MaxAttempts);
	return false;
}

//~=============================================================================
// Enemy Initialization & Cleanup
//~=============================================================================

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

		// 렌더링 상태 강제 업데이트 및 재등록
		Enemy->GetMesh()->RegisterComponent(); // 렌더링 시스템에 메시를 다시 등록
	}

	// 애니메이션 설정
	if (Data->AnimationSet && Data->AnimationSet->AnimationClass)
	{
		Enemy->GetMesh()->SetAnimInstanceClass(Data->AnimationSet->AnimationClass);
	}

	// GAS 속성 초기화
	// ✅ 서버에서만 GAS 초기화
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
				ASC->SetNumericAttributeBase(AttributeSet->GetAttackDamageAttribute(), Data->AttackDamage);
				ASC->SetNumericAttributeBase(AttributeSet->GetAttackRangeAttribute(), Data->AttackRange);
			}

			// 어빌리티 부여
			ASC->ClearAllAbilities();
			for (const TSubclassOf<UGameplayAbility>& AbilityClass : Data->StartAbilities)
			{
				if (AbilityClass)
				{
					FGameplayAbilitySpec Spec(AbilityClass, 1, INDEX_NONE, Enemy);
					ASC->GiveAbility(Spec);
				}
			}

			// GameplayEffect 적용
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
}

void UMSEnemySpawnSubsystem::ActivateEnemy(AMSBaseEnemy* Enemy, const FVector& Location) const
{
	if (!Enemy)
	{
		return;
	}
	
	// ✅ 풀링 모드 해제
	if (AMSNormalEnemy* NormalEnemy = Cast<AMSNormalEnemy>(Enemy))
	{
		NormalEnemy->SetPoolingMode(false);
	}

	// Enemy->SetActorLocation(Location);
	// Enemy->SetActorRotation(FRotator::ZeroRotator);
	// Enemy->SetActorHiddenInGame(false);
	// //Enemy->SetActorTickEnabled(true);  // 틱 활성화 추가
    
	// ✅ 2. 네트워크 상태 활성화
	Enemy->SetNetDormancy(DORM_Awake);
	Enemy->FlushNetDormancy();
	Enemy->SetReplicateMovement(true);
    
	// ✅ 3. 위치 설정
	Enemy->SetActorLocation(Location);
	Enemy->SetActorRotation(FRotator::ZeroRotator);
    
	// ✅ 4. 가시성/충돌 활성화
	Enemy->SetActorHiddenInGame(false);
	Enemy->SetActorEnableCollision(true);
	
	// ✅ AI Controller 생성 (수동)
	if (!Enemy->GetController())
	{
		Enemy->SpawnDefaultController();
	}
    
	// ✅ 5. 네트워크 업데이트 강제
	Enemy->ForceNetUpdate();
	
	// ✅ 디버그 로그
	UE_LOG(LogTemp, Error, TEXT("★★★ ActivateEnemy: %s | bReplicates: %d | Dormancy: %d ★★★"),
		*Enemy->GetName(),
		Enemy->GetIsReplicated(),
		(int32)Enemy->NetDormancy
	);

	// ✅ RVO 재활성화 (핵심!)
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

	// ✅ 1. Hidden 처리
	Enemy->SetActorHiddenInGame(true);
	Enemy->SetActorEnableCollision(false);
	Enemy->SetActorTickEnabled(false);
	
	// ✅ Movement 정리
	UCharacterMovementComponent* MovementComp = Enemy->GetCharacterMovement();
	if (MovementComp)
	{
		MovementComp->StopMovementImmediately();
		MovementComp->Velocity = FVector::ZeroVector;
		// RVO 비활성화 (다음 활성화 시 재등록됨)
		MovementComp->SetAvoidanceEnabled(false);
	}
	
	// AI 정지
	if (AController* Controller = Enemy->GetController())
	{
		if (AMSBaseAIController* AIController = Cast<AMSBaseAIController>(Controller))
		{
			AIController->StopAI();
		}
	}
	
	// ✅ 4. GAS 초기화
	ResetEnemyGASState(Enemy);
    
	// ✅ 5. 리플리케이션 끄기 (클라이언트에서 사라짐)
	// Enemy->SetReplicates(false);
	
	// ✅ 6. 위치는 그대로 두거나 원점으로
	// Enemy->SetActorLocation(FVector(0, 0, 100.0f));  // 선택사항
    
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

	// 1. 모든 GameplayTag 제거
	FGameplayTagContainer AllTags;
	ASC->GetOwnedGameplayTags(AllTags);
	ASC->RemoveLooseGameplayTags(AllTags);

	// 2. 모든 활성 GameplayEffect 제거
	ASC->RemoveActiveEffects(FGameplayEffectQuery());

	// 3. 모든 Ability 취소
	ASC->CancelAllAbilities();
}

//~=============================================================================
// Death Event Handling
//~=============================================================================

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
	if (NewCount > 0) // 태그 추가됨 = 사망
	{
		HandleEnemyDeath(Enemy);
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

//~=============================================================================
// Configuration Setters
//~=============================================================================

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
	MonsterPoolMap.Empty();

	// 새 DataTable 할당
	MonsterStaticDataTable = NewDataTable;

	// 데이터 다시 로드
	LoadMonsterDataTable();

	// 풀 재생성
	PrewarmPools();

	UE_LOG(LogTemp, Log, TEXT("[MonsterSpawn] DataTable reloaded successfully"));
}

//~=============================================================================
// Helper Functions
//~=============================================================================

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

//~=============================================================================
// Static Accessor
//~=============================================================================

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
