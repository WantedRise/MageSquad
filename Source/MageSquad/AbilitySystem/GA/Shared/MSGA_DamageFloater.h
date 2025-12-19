// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "MSGA_DamageFloater.generated.h"

/**
 * 작성자: 김준형
 * 작성일: 25/12/19
 *
 * 피해를 입으면, 받은 피해량만큼 대미지 플로터를 표시
 */
UCLASS()
class MAGESQUAD_API UMSGA_DamageFloater : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UMSGA_DamageFloater();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	// 어빌리티를 활성화해도 되는지 확인하는 헬퍼 함수
	bool CheckAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData);

protected:
	// 대미지 플로터 GameplayCue
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Ability")
	FGameplayTag Cue_DamageFloater;

};
