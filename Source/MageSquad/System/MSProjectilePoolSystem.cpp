//// Fill out your copyright notice in the Description page of Project Settings.
//
//
//#include "System/MSProjectilePoolSystem.h"
//#include "Actors/Projectile/MSBaseProjectile.h"
//#include "GameFramework/ProjectileMovementComponent.h"
//
//#include "Engine/GameInstance.h"
//#include "Engine/World.h"
//
//void UMSProjectilePoolSystem::Initialize(FSubsystemCollectionBase& Collection)
//{
//	Super::Initialize(Collection);
//
//	// 맵 초기화
//	ProjectilePools.Empty();
//	ProjectileToClassMap.Empty();
//}
//
//void UMSProjectilePoolSystem::Deinitialize()
//{
//	// 풀링된 발사체들을 모두 Destroy
//	for (auto& Pair : ProjectilePools)
//	{
//		for (AMSBaseProjectile* Projectile : Pair.Value.Pool)
//		{
//			if (IsValid(Projectile))
//			{
//				Projectile->Destroy();
//			}
//		}
//	}
//
//	// 맵 초기화
//	ProjectilePools.Empty();
//	ProjectileToClassMap.Empty();
//
//	Super::Deinitialize();
//}
//
//UMSProjectilePoolSystem* UMSProjectilePoolSystem::GetProjectilePoolSystem(UObject* WorldContextObject)
//{
//	if (!WorldContextObject) return nullptr;
//
//	if (UWorld* World = WorldContextObject->GetWorld())
//	{
//		if (UGameInstance* GI = World->GetGameInstance())
//		{
//			return GI->GetSubsystem<UMSProjectilePoolSystem>();
//		}
//	}
//	return nullptr;
//}
//
//void UMSProjectilePoolSystem::Configure(TSubclassOf<AMSBaseProjectile> InProjectileClass, int32 InInitialSize, int32 InBudget)
//{
//	if (!InProjectileClass) return;
//
//	FProjectilePoolData& Data = ProjectilePools.FindOrAdd(InProjectileClass);
//
//	// 초기 풀링 규모나 예산을 늘리는 경우
//	Data.InitialSize = FMath::Max(Data.InitialSize, InInitialSize);
//	Data.Budget = FMath::Max(Data.Budget, InBudget);
//
//	// 풀 초기화
//	PrewarmPool(InProjectileClass);
//}
//
//AMSBaseProjectile* UMSProjectilePoolSystem::SpawnProjectile(TSubclassOf<AMSBaseProjectile> InProjectileClass, const FTransform& Transform, AActor* Instigator)
//{
//	if (!InProjectileClass) return nullptr;
//
//	FProjectilePoolData* Data = ProjectilePools.Find(InProjectileClass);
//	if (!Data)
//	{
//		// 이 발사체 클래스의 풀 데이터가 없는 경우, 기본값으로 풀 생성
//		Configure(InProjectileClass, 8, 200);
//		Data = ProjectilePools.Find(InProjectileClass);
//	}
//	if (!Data) return nullptr;
//
//	// 최대 발사체 수가 넘어가는 경우 종료
//	if (Data->ActiveCount >= Data->Budget)
//	{
//		return nullptr;
//	}
//
//	// 생성된 발사체
//	AMSBaseProjectile* Projectile = nullptr;
//
//	// 풀에서 사용 가능한 발사체 찾기
//	for (int32 i = 0; i < Data->Pool.Num(); ++i)
//	{
//		AMSBaseProjectile* Candidate = Data->Pool[i];
//		if (IsValid(Candidate))
//		{
//			Projectile = Candidate;
//			Data->Pool.RemoveAtSwap(i);
//			break;
//		}
//	}
//
//	// 사용 가능한 인스턴스가 없으면 새로 생성
//	if (!Projectile)
//	{
//		Projectile = CreateNewProjectile(InProjectileClass, Instigator);
//	}
//	if (!Projectile) return nullptr;
//
//	// 활성화 카운트 증가 및 맵에 저장
//	Data->ActiveCount++;
//	ProjectileToClassMap.Add(Projectile, InProjectileClass);
//
//	// Instigator 설정
//	if (Instigator)
//	{
//		Projectile->SetInstigator(Cast<APawn>(Instigator));
//	}
//
//	// 트랜스폼 설정 및 발사체 활성화 설정
//	Projectile->SetActorHiddenInGame(false);
//	Projectile->SetActorEnableCollision(true);
//	Projectile->SetLifeSpan(0.f);
//	Projectile->SetActorTransform(Transform);
//	return Projectile;
//}
//
//void UMSProjectilePoolSystem::ReturnProjectile(AMSBaseProjectile* InProjectile)
//{
//	if (!InProjectile) return;
//
//	// 발사체 비활성화
//	InProjectile->SetActorHiddenInGame(true);
//	InProjectile->SetActorEnableCollision(false);
//
//	// 타이머 멈추기
//	if (UWorld* World = InProjectile->GetWorld())
//	{
//		World->GetTimerManager().ClearAllTimersForObject(InProjectile);
//	}
//
//	// 발사체 무브먼트 비활성화
//	if (UProjectileMovementComponent* MoveComp = InProjectile->FindComponentByClass<UProjectileMovementComponent>())
//	{
//		MoveComp->Deactivate();
//	}
//
//	// 풀 클래스 찾기
//	if (TSubclassOf<AMSBaseProjectile>* ClassPtr = ProjectileToClassMap.Find(InProjectile))
//	{
//		TSubclassOf<AMSBaseProjectile> ClassKey = *ClassPtr;
//
//		// 활성화 수 줄이기
//		if (FProjectilePoolData* Data = ProjectilePools.Find(ClassKey))
//		{
//			Data->ActiveCount = FMath::Max(0, Data->ActiveCount - 1);
//			Data->Pool.Add(InProjectile);
//		}
//		ProjectileToClassMap.Remove(InProjectile);
//	}
//	else
//	{
//		// 예외 발사체의 경우 제거
//		InProjectile->Destroy();
//	}
//}
//
//void UMSProjectilePoolSystem::SetProjectileBudget(TSubclassOf<AMSBaseProjectile> InProjectileClass, int32 NewBudget)
//{
//	if (!InProjectileClass || NewBudget < 0) return;
//
//	// 예산 초기화
//	FProjectilePoolData& Data = ProjectilePools.FindOrAdd(InProjectileClass);
//	Data.Budget = NewBudget;
//}
//
//AMSBaseProjectile* UMSProjectilePoolSystem::CreateNewProjectile(TSubclassOf<AMSBaseProjectile> InProjectileClass, AActor* Instigator)
//{
//	UWorld* World = GetWorld();
//	if (!World || !InProjectileClass) return nullptr;
//
//	// 스폰 파라미터 설정
//	FActorSpawnParameters Params;
//	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
//
//	// 새 발사체 생성
//	AMSBaseProjectile* NewProjectile = World->SpawnActor<AMSBaseProjectile>(InProjectileClass, FTransform::Identity, Params);
//	if (NewProjectile)
//	{
//		// 리플리케이션 비활성화
//		NewProjectile->SetReplicates(false);
//		NewProjectile->SetReplicateMovement(false);
//
//		// 발사체 생명주기 종료 콜백 함수 바인딩
//		NewProjectile->OnProjectileFinished.AddUObject(this, &UMSProjectilePoolSystem::HandleProjectileFinished);
//
//		// 발사체 초기화 설정
//		NewProjectile->SetActorHiddenInGame(true);
//		NewProjectile->SetActorEnableCollision(false);
//	}
//	return NewProjectile;
//}
//
//void UMSProjectilePoolSystem::PrewarmPool(TSubclassOf<AMSBaseProjectile> InProjectileClass)
//{
//	if (!InProjectileClass) return;
//
//	FProjectilePoolData* Data = ProjectilePools.Find(InProjectileClass);
//	if (!Data) return;
//
//	UWorld* World = GetWorld();
//	if (!World) return;
//
//	// 초기 인스턴스를 생성하여 풀 초기화
//	int32 CurrentCount = Data->Pool.Num() + Data->ActiveCount;
//	for (int32 i = CurrentCount; i < Data->InitialSize; ++i)
//	{
//		AMSBaseProjectile* NewProj = CreateNewProjectile(InProjectileClass, nullptr);
//		if (NewProj)
//		{
//			Data->Pool.Add(NewProj);
//		}
//	}
//}
//
//void UMSProjectilePoolSystem::HandleProjectileFinished(AMSBaseProjectile* InProjectile)
//{
//	ReturnProjectile(InProjectile);
//}
