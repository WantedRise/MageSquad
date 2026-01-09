// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/MSBaseEnemy.h"
#include "MSEliteEnemy.generated.h"

/**
 * 작성자 : 임희섭
 * 작성일 : 2026/01/09
 * 피통이 많은 특별 몬스터
 */
UCLASS()
class MAGESQUAD_API AMSEliteEnemy : public AMSBaseEnemy
{
	GENERATED_BODY()
	
public:
	AMSEliteEnemy();

protected:
	virtual void BeginPlay() override;	
	virtual void PossessedBy(AController* NewController) override;
	virtual void PostInitializeComponents() override;	
};

