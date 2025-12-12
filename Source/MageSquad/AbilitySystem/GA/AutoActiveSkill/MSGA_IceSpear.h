// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "MSGA_IceSpear.generated.h"

/**
 * 작성자: 박세찬
 * 작성일: 25/12/11
 * 
 * 얼음 결정 투사체를 발사하는 스킬
 */
UCLASS()
class MAGESQUAD_API UMSGA_IceSpear : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UMSGA_IceSpear();

	// 어빌리티 실행
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	/** 스킬 데이터 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Data")
	UDataTable* SkillDataTable;

	// 투사체 속도
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
	float CurrentProjectileSpeed = 3000.f;

	// 투사체 지속 시간
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
	float CurrentProjectileLifeTime = 3.f;

	// 투사체 개수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
	int32 CurrentProjectileNumber = 1;

	// 관통 횟수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
	int32 CurrentPenetration = 0;

	// 데미지
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
	float CurrentSkillDamage = 20.f;

	// 쿨타임
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
	float CoolTime = 3.f;
};
