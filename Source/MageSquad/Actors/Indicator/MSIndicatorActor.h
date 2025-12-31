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
 * 게임 플레이 큐에서 스폰할 예정
 */

UCLASS()
class MAGESQUAD_API AMSIndicatorActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMSIndicatorActor();
	
	void Initialize(const FAttackIndicatorParams& Params);

protected:
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UDecalComponent> DecalComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Indicator")
	TObjectPtr<UMaterialInterface> IndicatorMaterial;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

	UPROPERTY(ReplicatedUsing = OnRep_CachedParams)
	FAttackIndicatorParams CachedParams;

	float ElapsedTime = 0.f;

private:
	UFUNCTION()
	void OnRep_CachedParams();

	void ApplyMaterialParams();
	void UpdateFillPercent() const;

};
