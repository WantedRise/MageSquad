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

	/**
	 * Indicator 파라미터로 초기화
	 * @param Params - Indicator와 동일한 파라미터
	 * @param SourceASC - 데미지를 가하는 ASC
	 * @param DamageEffectClass - 적용할 GameplayEffect
	 */
	virtual void InitializeFromIndicator(
		const FAttackIndicatorParams& Params,
		UAbilitySystemComponent* SourceASC,
		TSubclassOf<UGameplayEffect> DamageEffectClass);

	// 즉시 충돌 검사 수행 후 결과 반환
	UFUNCTION(BlueprintCallable, Category = "Targeting")
	virtual TArray<AActor*> PerformTargeting();

	// 감지된 대상에게 데미지 적용
	UFUNCTION(BlueprintCallable, Category = "Targeting")
	virtual void ApplyDamageToTargets(const TArray<AActor*>& Targets);

	// 충돌 검사 + 데미지 적용을 한번에 수행
	UFUNCTION(BlueprintCallable, Category = "Targeting")
	virtual TArray<AActor*> ExecuteTargetingAndDamage();

protected:
	// 서브클래스에서 구현할 실제 충돌 검사 로직
	virtual TArray<AActor*> PerformOverlapCheck() PURE_VIRTUAL(AMSTargetActor_IndicatorBase::PerformOverlapCheck, return TArray<AActor*>(););

	// 유효한 타겟인지 필터링
	virtual bool IsValidTarget(AActor* Actor) const;

	// Indicator 파라미터
	UPROPERTY(BlueprintReadOnly, Category = "Targeting")
	FAttackIndicatorParams CachedParams;

	// 데미지 처리용
	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> SourceAbilitySystemComponent;

	UPROPERTY()
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// 충돌 검사 대상 채널
	UPROPERTY(EditDefaultsOnly, Category = "Targeting|Collision")
	TEnumAsByte<ECollisionChannel> TargetCollisionChannel = ECC_Pawn;

	// 플레이어 태그 (충돌 필터링용)
	UPROPERTY(EditDefaultsOnly, Category = "Targeting|Collision")
	FName PlayerTag = TEXT("Player");

	// 디버그 드로우 활성화
	UPROPERTY(EditDefaultsOnly, Category = "Targeting|Debug")
	bool bDrawDebug = false;

	// 디버그 드로우 지속시간
	UPROPERTY(EditDefaultsOnly, Category = "Targeting|Debug", meta = (EditCondition = "bDrawDebug"))
	float DebugDrawDuration = 2.f;
};