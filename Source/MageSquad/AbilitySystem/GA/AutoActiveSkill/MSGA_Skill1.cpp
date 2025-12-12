// Fill out your copyright notice in the Description page of Project Settings.


#include "MSGA_Skill1.h"
#include "Actors/Projectile/MSBaseProjectile.h"
#include "System/MSProjectilePoolSystem.h"

UMSGA_Skill1::UMSGA_Skill1()
{
	// Ability 인스턴싱/네트워크 정책
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UMSGA_Skill1::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// 서버에서만 발사체 스폰하도록
	if (!HasAuthority(&ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, /*bReplicateEnd*/true, /*bWasCancelled*/false);
		return;
	}

	// 어빌리티를 실행한 클래스 정보 가져오기
	// 정보를 가져오지 못했다면 경고로그와 함께 어빌리티 종료
	AActor* Avatar = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
	if (!Avatar)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMSGA_Skill1: Avatar is NULL"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// 발사 방향 (캐릭터 정면)
	const FVector Forward = Avatar->GetActorForwardVector().GetSafeNormal();
	const FVector_NetQuantize NetDirection = Forward;

	// 스폰 위치/변환 계산
	FVector SpawnLocation = Avatar->GetActorLocation();
	SpawnLocation += Forward * SpawnForwardOffset;
	SpawnLocation.Z += 50.0f;

	const FRotator SpawnRotation = Forward.Rotation();

	const FTransform SpawnTransform(SpawnRotation, SpawnLocation, FVector::OneVector);

	// Actor 스폰 파라미터
	FActorSpawnParameters Params;
	Params.Owner = Avatar;
	Params.Instigator = Cast<APawn>(Avatar);

	UMSProjectilePoolSystem* Pool = UMSProjectilePoolSystem::GetProjectilePoolSystem(Avatar);
	if (Pool)
	{
		AMSBaseProjectile* Projectile;

		Projectile = Pool->SpawnProjectile(SpawnTransform, Avatar);

		if (Projectile)
		{
			Projectile->InitProjectile(
				SpawnTransform,
				NetDirection,
				ProjectileSpeed,
				ProjectileLifeTime
			);
			UE_LOG(LogTemp, Log, TEXT("UMSGA_Skill1: Succeed to spawn projectile"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UMSGA_Skill1: Failed to spawn projectile"));
			EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
			return;
		}
	}

	// 어빌리티 종료
	EndAbility(Handle, ActorInfo, ActivationInfo, /*bReplicateEnd*/true, /*bWasCancelled*/false);
}
