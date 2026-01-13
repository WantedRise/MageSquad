// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Types/MSIndicatorTypes.h"
#include "MSIndicatorActor.generated.h"

/*
 * 작성자 : 임희섭
 * 작성일 : 2025/12/31
 * Indicator Decal을 표현할 액터
 * Fill 완료 시 Shape에 맞는 TargetActor를 스폰하여 충돌 검사 위임
 */

// Indicator 완료 시 Hit된 Actor 정보를 전달하는 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnIndicatorComplete, AMSIndicatorActor*, Indicator, const TArray<AActor*>&, HitActors);

UCLASS()
class MAGESQUAD_API AMSIndicatorActor : public AActor
{
	GENERATED_BODY()

public:
	AMSIndicatorActor();

	/**
	 * Indicator 초기화
	 * @param Params - Shape, Duration, Radius 등 설정
	 */
	void Initialize(const FAttackIndicatorParams& Params);

	/**
	 * 데미지 처리를 위한 추가 초기화
	 * @param SourceASC - 데미지를 가하는 ASC (보스)
	 * @param InDamageEffectClass - 적용할 GameplayEffect 클래스
	 */
	void SetDamageInfo(class UAbilitySystemComponent* SourceASC, TSubclassOf<class UGameplayEffect> InDamageEffectClass);

	// Indicator 완료 시 호출되는 델리게이트
	UPROPERTY(BlueprintAssignable)
	FOnIndicatorComplete OnIndicatorComplete;

	// 현재 파라미터 Getter
	FORCEINLINE const FAttackIndicatorParams& GetParams() const { return CachedParams; }

	void SetIndicatorOwner(AActor* InOwner);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayIndicatorCompleteCue(UAbilitySystemComponent* ASC, UParticleSystem* Particle, USoundBase* Sound);

protected:
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UFUNCTION()
	void OnRep_CachedParams();

	void ApplyMaterialParams();
	void UpdateFillPercent() const;

	// Fill 완료 시 호출
	void OnFillComplete();
	
	bool GetDamageInfo(UAbilitySystemComponent*& OutASC, TSubclassOf<UGameplayEffect>& OutDamageEffect) const;
	
protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UDecalComponent> DecalComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Indicator")
	TObjectPtr<UMaterialInterface> IndicatorMaterial;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

	UPROPERTY(ReplicatedUsing = OnRep_CachedParams)
	FAttackIndicatorParams CachedParams;

	// ===== TargetActor 클래스 (Shape별) =====
	UPROPERTY(EditDefaultsOnly, Category = "Indicator|TargetActor")
	TSubclassOf<class AMSTargetActor_IndicatorBase> SphereTargetActorClass;

	UPROPERTY(EditDefaultsOnly, Category = "Indicator|TargetActor")
	TSubclassOf<class AMSTargetActor_IndicatorBase> ConeTargetActorClass;

	UPROPERTY(EditDefaultsOnly, Category = "Indicator|TargetActor")
	TSubclassOf<class AMSTargetActor_IndicatorBase> RectangleTargetActorClass;

	// 데미지 처리용
	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> SourceAbilitySystemComponent;

	UPROPERTY()
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	float ElapsedTime = 0.f;
	
private:
	// Shape에 맞는 TargetActor 스폰 및 충돌 검사 수행
	TArray<AActor*> SpawnTargetActorAndPerformTargeting();

	// Shape별 TargetActor 클래스 반환
	TSubclassOf<class AMSTargetActor_IndicatorBase> GetTargetActorClassForShape() const;
	
	// 멤버 변수 추가
	UPROPERTY()
	TWeakObjectPtr<AActor> IndicatorOwner;
};