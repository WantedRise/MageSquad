// Fill out your copyright notice in the Description page of Project Settings.


#include "MSFunctionLibrary.h"

#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/ASC/MSPlayerAbilitySystemComponent.h"
#include "AbilitySystem/AttributeSets/MSPlayerAttributeSet.h"
#include "AbilitySystemBlueprintLibrary.h"

#include "Actors/Projectile/MSBaseProjectile.h"

#include "Player/MSPlayerState.h"

#include "MSGameplayTags.h"
#include "Actors/Projectile/MSEnemyProjectile.h"

UMSPlayerAbilitySystemComponent* UMSFunctionLibrary::NativeGetPlayerAbilitySystemComponentFromActor(AActor* InActor)
{
	if (!InActor) return nullptr;

	// #1: Actor가 ASC를 제공하는 경우
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(InActor))
	{
		return Cast<UMSPlayerAbilitySystemComponent>(ASI->GetAbilitySystemComponent());
	}

	// #2: Pawn이면 PlayerState에서 ASC 찾기
	if (APawn* Pawn = Cast<APawn>(InActor))
	{
		if (AMSPlayerState* MSPS = Cast<AMSPlayerState>(Pawn->GetPlayerState()))
		{
			return Cast<UMSPlayerAbilitySystemComponent>(MSPS->GetAbilitySystemComponent());
		}
	}

	// #3: Controller면 PlayerState에서 ASC 찾기
	if (AController* PC = Cast<AController>(InActor))
	{
		if (AMSPlayerState* MSPS = Cast<AMSPlayerState>(PC->PlayerState))
		{
			return Cast<UMSPlayerAbilitySystemComponent>(MSPS->GetAbilitySystemComponent());
		}
	}

	// #4: PlayerState 자체라면 ASC 반환
	if (AMSPlayerState* MSPS = Cast<AMSPlayerState>(InActor))
	{
		return Cast<UMSPlayerAbilitySystemComponent>(MSPS->GetAbilitySystemComponent());
	}
	
	return nullptr;
}

bool UMSFunctionLibrary::NativeDoesActorHaveTag(AActor* InActor, const FGameplayTag TagToCheck)
{
	if (UMSPlayerAbilitySystemComponent* ASC = NativeGetPlayerAbilitySystemComponentFromActor(InActor))
	{
		return ASC->HasMatchingGameplayTag(TagToCheck);
	}
	return false;
}

const UProjectileStaticData* UMSFunctionLibrary::GetProjectileStaticData(TSubclassOf<UProjectileStaticData> ProjectileDataClass)
{
	if (IsValid(ProjectileDataClass))
	{
		// 클래스의 CDO 반환
		// 클래스의 기본 객체를 반환하므로, 여러 개의 발사체가 같은 데이터 객체를 공유
		// 즉, 메모리에 새로운 객체를 생성하지 않고, 미리 존재하는 기본 객체를 사용
		return GetDefault<UProjectileStaticData>(ProjectileDataClass);
	}
	return nullptr;
}

FProjectileRuntimeData UMSFunctionLibrary::MakeProjectileRuntimeData(TSubclassOf<UProjectileStaticData> ProjectileDataClass)
{
	// 발사체의 런타임 데이터 생성
	FProjectileRuntimeData Data;

	// 발사체의 원본 데이터를 기준으로 모든 데이터 복사
	Data.CopyFromStaticData(GetProjectileStaticData(ProjectileDataClass));
	return Data;
}

AMSBaseProjectile* UMSFunctionLibrary::LaunchProjectileNative(UObject* WorldContextObject, TSubclassOf<UProjectileStaticData> ProjectileDataClass, FTransform Transform, AActor* Owner, APawn* Instigator, TSubclassOf<AMSBaseProjectile> ProjectileClass )
{
	// 발사한 객체의 월드 가져오기
	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	
	TSubclassOf<UProjectileStaticData> SpawnProjectileClass{ProjectileClass};
	
	if (SpawnProjectileClass == nullptr)
	{
		SpawnProjectileClass = AMSBaseProjectile::StaticClass();	
	}

	// 서버에서만 로직을 수행하도록 검사
	if (World && World->GetNetMode() < ENetMode::NM_Client)
	{
		// SpawnActorDeferred 함수를 통해, 발사체 지연 생성
		if (AMSBaseProjectile* Projectile = World->SpawnActorDeferred<AMSBaseProjectile>(SpawnProjectileClass, Transform, Owner, Instigator, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn))
		{
			// 발사체 정적 데이터 클래스를 초기화
			Projectile->ProjectileDataClass = ProjectileDataClass;
			Projectile->InitProjectileRuntimeDataFromClass(ProjectileDataClass);
			Projectile->FinishSpawning(Transform);

			return Projectile;
		}
	}

	return nullptr;
}

AMSBaseProjectile* UMSFunctionLibrary::LaunchProjectile(UObject* WorldContextObject, TSubclassOf<UProjectileStaticData> ProjectileDataClass, FProjectileRuntimeData RuntimeData, FTransform Transform, AActor* Owner, APawn* Instigator, TSubclassOf<AMSBaseProjectile> ProjectileClass)
{
	// 발사한 객체의 월드 가져오기
	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	
	TSubclassOf<AMSBaseProjectile> SpawnProjectileClass{ProjectileClass};
	
	if (SpawnProjectileClass == nullptr)
	{
		SpawnProjectileClass = AMSBaseProjectile::StaticClass();	
	}

	// 서버에서만 로직을 수행하도록 검사
	if (World && World->GetNetMode() < ENetMode::NM_Client)
	{
		// SpawnActorDeferred 함수를 통해, 발사체 지연 생성
		if (AMSBaseProjectile* Projectile = World->SpawnActorDeferred<AMSBaseProjectile>(SpawnProjectileClass, Transform, Owner, Instigator, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn))
		{
			// 발사체 정적 데이터 클래스를 초기화
			Projectile->ProjectileDataClass = ProjectileDataClass;
			Projectile->SetProjectileRuntimeData(RuntimeData);
			Projectile->FinishSpawning(Transform);
			return Projectile;
		}
	}

	return nullptr;
}