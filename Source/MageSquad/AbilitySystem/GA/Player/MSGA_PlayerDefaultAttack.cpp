// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GA/Player/MSGA_PlayerDefaultAttack.h"
#include "Actors/Projectile/MSBaseProjectile.h"
#include "AbilitySystemComponent.h"
#include "Player/MSPlayerCharacter.h"

#include "NiagaraComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "GameFramework/PlayerController.h"

#include "System/MSVFXSFXBudgetSystem.h"
#include "System/MSProjectilePoolSystem.h"

UMSGA_PlayerDefaultAttack::UMSGA_PlayerDefaultAttack()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// 변수 초기화
	AutoAttackInterval = 0.6f;   // 자동 공격 주기
	ProjectileSpeed = 1200.f;
	ProjectileLifeTime = 3.0f;
}

void UMSGA_PlayerDefaultAttack::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	// 클라이언트 로직
	if (ActorInfo && ActorInfo->IsLocallyControlled() && ActorInfo->AbilitySystemComponent.IsValid())
	{
		// 클라이언트에서 어빌리티 자동 활성화
		ActorInfo->AbilitySystemComponent.Get()->TryActivateAbility(Spec.Handle);
	}
}

void UMSGA_PlayerDefaultAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!ActorInfo)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// 로컬에서 제어하는 클라이언트에서만 자동 공격 타이머 시작
	if (ActorInfo->IsLocallyControlled())
	{
		// 자동 공격 세팅
		if (UWorld* World = ActorInfo->AvatarActor->GetWorld())
		{
			World->GetTimerManager().ClearTimer(AutoAttackTimerHandle);
			World->GetTimerManager().SetTimer(
				AutoAttackTimerHandle,
				this,
				&UMSGA_PlayerDefaultAttack::PerformAutoAttack,
				AutoAttackInterval,
				true,
				0.0f
			);
		}
	}
}

void UMSGA_PlayerDefaultAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 타이머 클리어
	if (ActorInfo && ActorInfo->IsLocallyControlled())
	{
		if (UWorld* World = ActorInfo->AvatarActor->GetWorld())
		{
			World->GetTimerManager().ClearTimer(AutoAttackTimerHandle);
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UMSGA_PlayerDefaultAttack::PerformAutoAttack()
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid()) return;

	if (!ActorInfo->IsLocallyControlled()) return;

	// 플레이어 캐릭터 가져오기
	AMSPlayerCharacter* Player = Cast<AMSPlayerCharacter>(ActorInfo->AvatarActor.Get());
	APlayerController* PC = Cast<APlayerController>(ActorInfo->PlayerController.Get());
	if (!Player || !PC)
	{
		return;
	}

	// 마우스 커서 아래 월드 위치 계산
	FHitResult Hit;
	if (!PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit))
	{
		return;
	}

	const FVector SpawnLocation = Player->GetActorLocation() + FVector(0.f, 0.f, 50.f);
	FVector Direction = Hit.Location - SpawnLocation;
	if (!Direction.Normalize())
	{
		Direction = Player->GetActorForwardVector();
	}

	// 서버에 발사 요청
	ServerRPCPerformAutoAttack(SpawnLocation, Direction, ProjectileSpeed, ProjectileLifeTime);
}

void UMSGA_PlayerDefaultAttack::ServerRPCPerformAutoAttack_Implementation(const FVector_NetQuantize& SpawnLocation, const FVector_NetQuantizeNormal& Direction, float InProjectileSpeed, float InProjectileLifeTime)
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid()) return;

	AActor* Avatar = ActorInfo->AvatarActor.Get();
	UWorld* World = Avatar->GetWorld();
	if (!World) return;

	// 발사체 풀링 시스템 가져오기
	UMSProjectilePoolSystem* PoolSystem = UMSProjectilePoolSystem::GetProjectilePoolSystem(Avatar);
	UMSVFXSFXBudgetSystem* BudgetSystem = UMSVFXSFXBudgetSystem::GetVFXSFXBudgetSystem(Avatar);
	if (!PoolSystem || !BudgetSystem) return;

	// 스폰 트랜스폼 구하기
	const FRotator SpawnRot = Direction.ToOrientationRotator();
	const FTransform SpawnTransform(SpawnRot, SpawnLocation);

	// 발사체 풀링 시스템에서 재사용 발사체 가져오기
	AMSBaseProjectile* Spawned = PoolSystem->SpawnProjectile(SpawnTransform, Avatar);

	// 발사체 초기화
	if (Spawned)
	{
		Spawned->InitProjectile(SpawnTransform, Direction, InProjectileSpeed, InProjectileLifeTime);
	}

	//// VFX/SFX 재생
	//if (ProjectileVFX)
	//{
	//	UNiagaraComponent* SpawnVFX = Cast<UNiagaraComponent>(BudgetSystem->SpawnVFX(ProjectileVFX, SpawnTransform));
	//	SpawnVFX->SetupAttachment(Spawned->GetRootComponent());
	//}
	//if (ProjectileSFX)
	//{
	//	BudgetSystem->PlaySFX(ProjectileSFX, SpawnLocation);
	//}
}
