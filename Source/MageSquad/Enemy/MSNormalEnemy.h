// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/MSBaseEnemy.h"
#include "MSNormalEnemy.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API AMSNormalEnemy : public AMSBaseEnemy
{
	GENERATED_BODY()
public:
	AMSNormalEnemy();
	
public:
	virtual void BeginPlay() override;	
	virtual void PossessedBy(AController* NewController) override;
	
};
