// Fill out your copyright notice in the Description page of Project Settings.


#include "MSFunctionLibrary.h"
#include "AbilitySystem/ASC/MSPlayerAbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Actors/Projectile/MSBaseProjectile.h"

#include "MSGameplayTags.h"

UMSPlayerAbilitySystemComponent* UMSFunctionLibrary::NativeGetPlayerAbilitySystemComponentFromActor(AActor* InActor)
{
	if (!InActor)
	{
		return nullptr;
	}

	if (UMSPlayerAbilitySystemComponent* ASC = Cast<UMSPlayerAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InActor)))
	{
		return ASC;
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

AMSBaseProjectile* UMSFunctionLibrary::LaunchProjectile(UObject* WorldContextObject, TSubclassOf<UProjectileStaticData> ProjectileDataClass, FTransform Transform, AActor* Owner, APawn* Instigator)
{
	// 발사한 객체의 월드 가져오기
	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;

	// 서버에서만 로직을 수행하도록 검사
	if (World && World->GetNetMode() < ENetMode::NM_Client)
	{
		// SpawnActorDeferred 함수를 통해, 발사체 지연 생성
		if (AMSBaseProjectile* Projectile = World->SpawnActorDeferred<AMSBaseProjectile>(AMSBaseProjectile::StaticClass(), Transform, Owner, Instigator, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn))
		{
			// 발사체 정적 데이터 클래스를 초기화하고, 스폰 프로세스를 마무리
			Projectile->ProjectileDataClass = ProjectileDataClass;
			Projectile->FinishSpawning(Transform);

			return Projectile;
		}
	}

	return nullptr;
}
