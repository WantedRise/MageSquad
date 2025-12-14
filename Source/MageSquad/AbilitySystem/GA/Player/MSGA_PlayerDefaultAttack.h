// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "MSGA_PlayerDefaultAttack.generated.h"

/**
 * 작성자: 김준형
 * 작성일: 25/12/15
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

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	// 발사체 데이터 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Ability")
	TSubclassOf<class UProjectileStaticData> ProjectileDataClass;
};
