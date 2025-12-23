// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GA/Enemy/MSGA_EnemyShootProjectile.h"

#include "MSFunctionLibrary.h"
#include "Actors/Projectile/Behaviors/MSProjectileBehavior_Normal.h"
#include "Types/MageSquadTypes.h"

UMSGA_EnemyShootProjectile::UMSGA_EnemyShootProjectile()
{
}

void UMSGA_EnemyShootProjectile::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	// FProjectileRuntimeData RuntimeData;
	// RuntimeData.BehaviorClass = UMSProjectileBehavior_Normal::StaticClass();
	//
	// AActor* CachedAvatar = GetAvatarActorFromActorInfo();
	//
	// UMSFunctionLibrary::LaunchProjectile(
	// 	this,
	// 	ProjectileDataClass,
	// 	RuntimeData,
	// 	CachedSpawnTransform, // 스폰 시킬 Transform
	// 	CachedAvatar,
	// 	Cast<APawn>(CachedAvatar)
	// );
}

void UMSGA_EnemyShootProjectile::CancelAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UMSGA_EnemyShootProjectile::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UMSGA_EnemyShootProjectile::OnCompleteCallback()
{
}

void UMSGA_EnemyShootProjectile::OnInterruptedCallback()
{
}
