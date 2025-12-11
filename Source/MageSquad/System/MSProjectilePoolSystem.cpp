// Fill out your copyright notice in the Description page of Project Settings.


#include "System/MSProjectilePoolSystem.h"
#include "Actors/Projectile/MSBaseProjectile.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"

void UMSProjectilePoolSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	InitialPoolSize = 32;

	/*
	* GameMode/GameInstance 등 게임 로직 시작 시에 Configure 함수 호출해서 풀링 초기화하기
	*/
}

void UMSProjectilePoolSystem::Deinitialize()
{
	// 발사체 풀링 리스트 초기화
	FreeProjectiles.Empty();

	Super::Deinitialize();
}

UMSProjectilePoolSystem* UMSProjectilePoolSystem::GetProjectilePoolSystem(UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;

	// 해당 월드 컨텍스트 오브젝트의 월드에서 이 서브시스템을 반환
	if (UWorld* World = WorldContextObject->GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			return GI->GetSubsystem<UMSProjectilePoolSystem>();
		}
	}

	return nullptr;
}

void UMSProjectilePoolSystem::Configure(TSubclassOf<class AMSBaseProjectile> InProjectileClass, int32 InInitialPoolSize)
{
	// 발사체 클래스 초기화 및 풀 사이즈 설정
	PoolProjectileClass = InProjectileClass;
	InitialPoolSize = FMath::Max(InitialPoolSize, InInitialPoolSize);

	// 풀 채우기
	PrewarmPool();
}

AMSBaseProjectile* UMSProjectilePoolSystem::SpawnProjectile(const FTransform& SpawnTransform, AActor* Instigator)
{
	if (!PoolProjectileClass || !Instigator) return nullptr;

	UWorld* World = GetTypeWorld();
	if (!World) return nullptr;

	// 반환할 발사체
	AMSBaseProjectile* Ret = nullptr;

	// 풀 배열에서 재사용할 수 있는 객체가 있으면 재사용
	while (FreeProjectiles.Num() > 0 && !Ret)
	{
		// 풀 배열에서 하나씩 꺼내며 확인
		AMSBaseProjectile* Candidate = FreeProjectiles.Pop();
		if (IsValid(Candidate))
		{
			Ret = Candidate;
		}
	}

	// 재사용할 객체가 없으면 새로 생성
	if (!Ret)
	{
		// 스폰 파라미터 설정 (항상 스폰)
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// 새 발사체를 스폰해서 저장
		Ret = World->SpawnActor<AMSBaseProjectile>(PoolProjectileClass, FTransform::Identity, Params);

		if (Ret)
		{
			// 발사체 비활성화
			Ret->SetActorHiddenInGame(true);
			Ret->SetActorEnableCollision(false);

			// 발사체 생명주기 종료 후, 풀로 되돌리는 함수 바인딩
			Ret->OnProjectileFinished.Clear();
			Ret->OnProjectileFinished.AddUObject(this, &UMSProjectilePoolSystem::HandleProjectileFinished);

			// 소유자 설정
			Ret->SetInstigator(Cast<APawn>(Instigator));
		}
	}

	return Ret;
}

void UMSProjectilePoolSystem::ReturnProjectile(AMSBaseProjectile* InProjectile)
{
	if (!InProjectile) return;

	// 발사체 비활성화
	InProjectile->SetActorHiddenInGame(true);
	InProjectile->SetActorEnableCollision(false);

	// 재사용 가능한 발사체 리스트에 추가
	FreeProjectiles.Add(InProjectile);
}

UWorld* UMSProjectilePoolSystem::GetTypeWorld() const
{
	return GetWorld();
}

void UMSProjectilePoolSystem::PrewarmPool()
{
	if (!PoolProjectileClass) return;

	UWorld* World = GetTypeWorld();
	if (!World) return;

	// 현재 풀 사이즈
	const int32 CurrentCount = FreeProjectiles.Num();
	if (CurrentCount >= InitialPoolSize) return;

	// 풀 사이즈만큼 빈 풀을 스폰해서 채움
	for (int32 i = CurrentCount; i < InitialPoolSize; ++i)
	{
		// 스폰 파라미터 설정 (항상 스폰)
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// 새 발사체 스폰
		AMSBaseProjectile* NewProjectile
			= World->SpawnActor<AMSBaseProjectile>(PoolProjectileClass, FTransform::Identity, Params);

		if (NewProjectile)
		{
			// 발사체 비활성화
			NewProjectile->SetActorHiddenInGame(true);
			NewProjectile->SetActorEnableCollision(false);

			// 발사체 생명주기 종료 후, 풀로 되돌리는 함수 바인딩
			NewProjectile->OnProjectileFinished.Clear();
			NewProjectile->OnProjectileFinished.AddUObject(this, &UMSProjectilePoolSystem::HandleProjectileFinished);

			FreeProjectiles.Add(NewProjectile);
		}
	}
}

void UMSProjectilePoolSystem::HandleProjectileFinished(AMSBaseProjectile* InProjectile)
{
	if (!InProjectile) return;
	ReturnProjectile(InProjectile);
}
