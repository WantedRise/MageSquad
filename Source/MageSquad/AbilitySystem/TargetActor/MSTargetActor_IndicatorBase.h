// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "Types/MSIndicatorTypes.h"
#include "MSTargetActor_IndicatorBase.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;
class AMSIndicatorActor;

/*
 * 작성자 : 임희섭
 * 작성일 : 2026/01/02
 * Indicator와 연동하는 TargetActor 베이스 클래스
 * Indicator의 Shape/Size 정보를 받아 동일한 범위로 충돌 검사 수행
 */
UCLASS(Abstract)
class MAGESQUAD_API AMSTargetActor_IndicatorBase : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()

public:
	AMSTargetActor_IndicatorBase();
	
	virtual void InitializeFromIndicator(const FAttackIndicatorParams& Params, UAbilitySystemComponent* SourceASC, TSubclassOf<UGameplayEffect> DamageEffectClass);

	UFUNCTION(BlueprintCallable, Category = "Targeting")
	virtual TArray<AActor*> PerformTargeting();

	UFUNCTION(BlueprintCallable, Category = "Targeting")
	virtual void ApplyDamageToTargets(const TArray<AActor*>& Targets);

	// 충돌 검사 + 데미지 적용을 한번에 수행
	UFUNCTION(BlueprintCallable, Category = "Targeting")
	virtual TArray<AActor*> ExecuteTargetingAndDamage();

protected:
	// 서브클래스에서 구현할 실제 충돌 검사 로직
	virtual TArray<AActor*> PerformOverlapCheck() PURE_VIRTUAL(AMSTargetActor_IndicatorBase::PerformOverlapCheck, return TArray<AActor*>(););

	UPROPERTY(BlueprintReadOnly, Category = "Targeting")
	FAttackIndicatorParams CachedParams;

	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> SourceAbilitySystemComponent;

	UPROPERTY()
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// 충돌 검사 대상 채널 : Player 채널로 지정
	UPROPERTY(EditDefaultsOnly, Category = "Targeting|Collision")
	TEnumAsByte<ECollisionChannel> TargetCollisionChannel = ECC_GameTraceChannel1;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting|Debug")
	bool bDrawDebug = false;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting|Debug", meta = (EditCondition = "bDrawDebug"))
	float DebugDrawDuration = 2.f;
};