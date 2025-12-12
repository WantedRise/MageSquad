// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/MSBaseEnemy.h"
#include "MSNormalEnemy.generated.h"

/**
 * 작성자 : 임희섭
 * 작성일 : 2025/12/12
 * 일반 몬스터들 클래스, 기본적인 동작만 가진다. 
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
	virtual void InitEnemyData(UDA_MonsterSpawnData* InEnemyData) override;
	
};
