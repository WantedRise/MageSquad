// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/MSBaseEnemy.h"
#include "MSNormalEnemy.generated.h"

/**
 * 작성자 : 임희섭
 * 작성일 : 2025/12/12
 * 일반 몬스터 클래스
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
	virtual void PostInitializeComponents() override;	

	// ✅ 추가: 풀링 모드 제어
	void SetPoolingMode(bool bInPooling);
	
private:
	// ✅ 풀링 중에는 Possess 안 함
	bool bIsInPool = false;

};
