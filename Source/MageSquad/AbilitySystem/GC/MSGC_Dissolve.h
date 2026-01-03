// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "MSGC_Dissolve.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UMSGC_Dissolve : public UGameplayCueNotify_Static
{
	GENERATED_BODY()
public:
	UMSGC_Dissolve();
	
public:
	virtual bool OnExecute_Implementation(
		AActor* Target,
		const FGameplayCueParameters& Parameters) const override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Dissolve")
	float DissolveDuration = 1.5f;
    
	UPROPERTY(EditDefaultsOnly, Category = "Dissolve")
	FName DissolveParameterName = FName("Dissolve");
	
	UPROPERTY(EditDefaultsOnly, Category = "Dissolve")
	FName DissolveValueParameterName = FName("DissolveValue");
};
