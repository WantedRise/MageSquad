// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Projectile/MSBaseProjectile.h"
#include "MSEnemyProjectile.generated.h"

/**
 * 작성자 : 임희섭
 * 작성일 : 2025/12/23
 * Enemy 용 Projectile 클래스
 */
UCLASS()
class MAGESQUAD_API AMSEnemyProjectile : public AMSBaseProjectile
{
	GENERATED_BODY()
	
public:
	AMSEnemyProjectile();
};
