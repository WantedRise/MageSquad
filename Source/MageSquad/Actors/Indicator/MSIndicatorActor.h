// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Types/MSIndicatorTypes.h"
#include "MSIndicatorActor.generated.h"

UCLASS()
class MAGESQUAD_API AMSIndicatorActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMSIndicatorActor();
	
	void Initialize(const FAttackIndicatorParams& Params);

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UDecalComponent> DecalComponent;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

	FAttackIndicatorParams CachedParams;
	float ElapsedTime = 0.f;

	virtual void Tick(float DeltaTime) override;

private:
	void UpdateFillPercent() const;

};
