// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GA/Skill/MSGA_SkillBase.h"
#include "MSGA_Explosion.generated.h"

/**
*  작성자: 박세찬
 * 작성일: 25/12/16
 * 
 * 우클릭으로 발동되는 스킬 중 하나
 * 마우스 위치에 폭발을 일으킴
 */
UCLASS()
class MAGESQUAD_API UMSGA_Explosion : public UMSGA_SkillBase
{
	GENERATED_BODY()
	
public:
	UMSGA_Explosion();
	
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
	
protected:
	// Explosion 스킬 ID는 21
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentSkillID = 21;
	
	// 피해량
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<float> BaseDamageSequence = { 0.1f, 0.1f, 0.1f, 0.7f };
	
	
	// 피해 주기
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DamageInterval = 0.25f;

	// 쿨타임
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CoolTime = 3.f;

	// 스킬 범위
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Range = 1.f;

	// 투사체 스태틱데이터 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<class UProjectileStaticData> ProjectileDataClass;

	// 틱 데미지 적용에 사용할 GE
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<class UGameplayEffect> DamageEffect;
};
