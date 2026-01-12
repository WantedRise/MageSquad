// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "MSGC_HitFlash.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UMSGC_HitFlash : public UGameplayCueNotify_Static
{
	GENERATED_BODY()
	
public:
	UMSGC_HitFlash();
	
public:
	virtual bool OnExecute_Implementation(
		AActor* Target,
		const FGameplayCueParameters& Parameters) const override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "HitFlash")
	float FlashDuration = 0.2f;
    
	UPROPERTY(EditDefaultsOnly, Category = "HitFlash")
	FLinearColor FlashColor = FLinearColor(1.f, 0.f, 0.f);
    
	UPROPERTY(EditDefaultsOnly, Category = "HitFlash")
	FName MaterialParameterName = FName("HitFlashIntensity");
	
};
