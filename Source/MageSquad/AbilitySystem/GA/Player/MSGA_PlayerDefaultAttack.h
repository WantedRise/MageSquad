// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "MSGA_PlayerDefaultAttack.generated.h"

/**
 * 작성자: 김준형
 * 작성일: 25/12/11
 *
 * 플레이어의 기본 공격 어빌리티 (자동 공격)
 * 마우스 커서로 매직 미사일 발사
 */
UCLASS()
class MAGESQUAD_API UMSGA_PlayerDefaultAttack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UMSGA_PlayerDefaultAttack();

	// 아바타 액터가 설정되거나 변경될 때 호출
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UFUNCTION()
	void PerformAutoAttack();

	// 서버에게 발사체 발사 요청 함수 (ServerRPC)
	UFUNCTION(Server, Reliable)
	void ServerRPCPerformAutoAttack(
		const FVector_NetQuantize& SpawnLocation,
		const FVector_NetQuantizeNormal& Direction,
		float InProjectileSpeed,
		float InProjectileLifeTime
	);

protected:
	// 발사체 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Ability")
	TSubclassOf<class AMSBaseProjectile> ProjectileClass;

	// 발사(공격) 주기
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Ability")
	float AutoAttackInterval = 1.f;

	// 발사체 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Ability")
	float ProjectileSpeed = 1.f;

	// 발사체 생명주기
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Ability")
	float ProjectileLifeTime = 1.f;

	// VFX
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Ability")
	TObjectPtr<class UFXSystemAsset> ProjectileVFX;

	// SFX
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Ability")
	TObjectPtr<class USoundBase> ProjectileSFX;

private:
	// 자동 공격 타이머 핸들
	FTimerHandle AutoAttackTimerHandle;
};
