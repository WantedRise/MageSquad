// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GA/AutoActiveSkill/MSGA_AutoActiveSkillBase.h"
#include "MSFunctionLibrary.h"
#include "MSGA_IceSpear.generated.h"

/**
*  작성자: 박세찬
 * 작성일: 25/12/12
 * 
 * 자동 발동되는 공통 스킬 중 하나
 * 가장 가까운 적에게 투사체를 날리는 스킬
 */
UCLASS()
class MAGESQUAD_API UMSGA_IceSpear : public UMSGA_AutoActiveSkillBase
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
	// Ice Spear 스킬 ID는 1
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentSkillID = 1;
	
	// 피해량
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SkillDamage = 20.f;

	// 쿨타임
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CoolTime = 3.f;

	// 투사체 개수
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ProjectileNumber = 1;

	// 관통 횟수
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Penetration = 0;
	
	// 발사체 원본 데이터 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<class UProjectileStaticData> ProjectileDataClass;
	
	// ===== 연사용 상태 변수 =====
	int32 FiredCount = 0;

	FVector CachedOrigin;
	FVector CachedDirection;
	FTransform CachedSpawnTransform;
	FProjectileRuntimeData CachedRuntimeData;
	TWeakObjectPtr<AActor> CachedAvatar;
	
	UFUNCTION()
	void FireNextProjectile();
	
	UFUNCTION()
	FVector FindClosestEnemyLocation(const UWorld* World, const AActor* Avatar) const;
};