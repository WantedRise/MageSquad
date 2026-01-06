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

// public: /*Setter*/
// 	FORCEINLINE void SetProjectileData(TSubclassOf<class UProjectileStaticData> InProjectileDataClass) { ProjectileDataClass = InProjectileDataClass;}
// 	
// private:
// 	// 발사체 원본 데이터 클래스
// 	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile", meta = (AllowPrivateAccess = "true"))
// 	TSubclassOf<class UProjectileStaticData> ProjectileDataClass;

};
