// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "MSGA_Skill1.generated.h"

/**
 * 작성자: 박세찬
 * 작성일: 25/12/11
 * 
 * 투사체를 발사하는 기본 스킬
 */
UCLASS()
class MAGESQUAD_API UMSGA_Skill1 : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UMSGA_Skill1();

	// Ability 실행
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	/** 발사체 속도 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Projectile")
	float ProjectileSpeed = 2000.f;

	/** 캐릭터와 얼마나 떨어진 곳에서 스폰할지 거리 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Projectile")
	float SpawnForwardOffset = 100.f;

	/** 발사체 생존 시간(초) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Projectile")
	float ProjectileLifeTime = 5.f;

	/** 발사체 개수 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Projectile")
	float ProjectileLifeTime = 1.f;
};
