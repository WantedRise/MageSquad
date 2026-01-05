// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GA/Skill/MSGA_SkillBase.h"
#include "MSFunctionLibrary.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Actors/Projectile/Behaviors/MSProjectileBehaviorBase.h"
#include "MSGA_IceSpear.generated.h"

/**
*  ?묒꽦?? 諛뺤꽭李?
 * ?묒꽦?? 25/12/12
 * 
 * ?먮룞 諛쒕룞?섎뒗 怨듯넻 ?ㅽ궗 以??섎굹
 * 媛??媛源뚯슫 ?곸뿉寃??ъ궗泥대? ?좊━???ㅽ궗
 */

UCLASS()
class MAGESQUAD_API UMSGA_IceSpear : public UMSGA_SkillBase
{
	GENERATED_BODY()
	
public:
	UMSGA_IceSpear();
	
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
	
protected:
	// Ice Spear ?ㅽ궗 ID??1
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentSkillID = 1;
	
	// ?쇳빐??
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SkillDamage = 20.f;

	// ?ъ궗泥?媛쒖닔
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ProjectileNumber = 1;

	// 愿???잛닔
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Penetration = 0;
	
	
	
	// ===== ?곗궗???곹깭 蹂??=====
	int32 FiredCount = 0;

	FVector CachedOrigin;
	FVector CachedDirection;
	FTransform CachedSpawnTransform;
	FProjectileRuntimeData CachedRuntimeData;
	TWeakObjectPtr<AActor> CachedAvatar;
	
	UFUNCTION()
	void FireNextProjectile();
	
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitDelay> FireDelayTask = nullptr;
	
	UFUNCTION()
	FVector FindClosestEnemyLocation(const UWorld* World, const AActor* Avatar) const;
};
