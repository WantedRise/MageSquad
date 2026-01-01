// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GA/Skill/MSGA_SkillBase.h"
#include "MSGA_Tornado.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UMSGA_Tornado : public UMSGA_SkillBase
{
	GENERATED_BODY()
	
public:
	UMSGA_Tornado();
	
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
	
protected:
	// Tornado 스킬 ID는 3
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentSkillID = 3;
	
	// 피해량
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SkillDamage = 10.f;
	
	// 범위
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SkillRadius = 200.f;
	
	// 지속시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SkillDuration = 8.f;
	
	// 발사체 원본 데이터 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<class UProjectileStaticData> ProjectileDataClass;
	
	// 데미지 전달 GE
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<UGameplayEffect> DamageEffect;
};
