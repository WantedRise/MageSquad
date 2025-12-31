// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GA/Skill/MSGA_SkillBase.h"
#include "MSGA_Lightning.generated.h"

/**
*  작성자: 박세찬
 * 작성일: 26/01/01
 * 
 * 몬스터 위치에 낙뢰
 */
UCLASS()
class MAGESQUAD_API UMSGA_Lightning : public UMSGA_SkillBase
{
	GENERATED_BODY()
	
public:
	UMSGA_Lightning(); 
	
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
	
protected:
	// Lightning 스킬 ID는 2
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentSkillID = 2;
	
	// 피해량
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SkillDamage = 20.f;

	// 투사체 개수
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ProjectileNumber = 1;
	
	// 발사체 원본 데이터 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<class UProjectileStaticData> ProjectileDataClass;
	
	// 데미지 전달 GE
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<UGameplayEffect> DamageEffect;
	
	// 범위 안의 랜덤 몬스터 선택
	UFUNCTION()
	void FindRandomEnemyTargets(
		const UWorld* World,
		const AActor* Avatar,
		int32 NumToPick,
		TArray<AActor*>& OutTargets
	) const;
};
