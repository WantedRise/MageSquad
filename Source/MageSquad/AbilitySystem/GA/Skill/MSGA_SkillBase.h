// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "SkillData/MSSkillList.h"
#include "MSGA_SkillBase.generated.h"

/**
 * 작성자: 박세찬
 * 작성일: 25/12/12
 * 
 * 자동 발동되는 공통 스킬들이 상속하는 베이스클래스
 * 데이터테이블을 불러오는 기능을 가짐
 */
UCLASS()
class MAGESQUAD_API UMSGA_SkillBase : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UMSGA_SkillBase();
	
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
	
protected:
	// 스킬 데이터
	UPROPERTY(EditDefaultsOnly, Category="SkillData")
	FMSSkillList SkillListRow;
	
	// 스킬 ID
	UPROPERTY(EditDefaultsOnly, Category="SkillData")
	int32 SkillID;
};
