// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataAssets/Enemy/DA_EnemyBossAnimationSet.h"
#include "Enemy/MSBaseEnemy.h"
#include "MSBossEnemy.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API AMSBossEnemy : public AMSBaseEnemy
{
	GENERATED_BODY()
	
public:
	AMSBossEnemy();
	
public:
	virtual void BeginPlay() override;	
	
	// 풀링 모드 제어
	virtual void SetPoolingMode(bool bInPooling) override;
	
public:
	FORCEINLINE UAnimMontage* GetSpawnMontage() const {return Cast<UDA_EnemyBossAnimationSet>(AnimData)->SpawnAnim;}
	FORCEINLINE UAnimMontage* GetGroggyMontage() const {return Cast<UDA_EnemyBossAnimationSet>(AnimData)->GroggyAnim;}
};
