// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GA/Skill/MSGA_SkillBase.h"
#include "MSGA_Lightning.generated.h"

/**
*  ?묒꽦?? 諛뺤꽭李?
 * ?묒꽦?? 26/01/01
 * 
 * 紐ъ뒪???꾩튂???숇ː
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
	// Lightning ?ㅽ궗 ID??2
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentSkillID = 2;
	
	// ?쇳빐??
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SkillDamage = 20.f;

	// ?ъ궗泥?媛쒖닔
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ProjectileNumber = 1;
	
	// 踰붿쐞
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SkillRadius = 200.f;
	
	
	
	// 踰붿쐞 ?덉쓽 ?쒕뜡 紐ъ뒪???좏깮
	UFUNCTION()
	void FindRandomEnemyTargets(
		const UWorld* World,
		const AActor* Avatar,
		int32 NumToPick,
		TArray<AActor*>& OutTargets
	) const;
};
