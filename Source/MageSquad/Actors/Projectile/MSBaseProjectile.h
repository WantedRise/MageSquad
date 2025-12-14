// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Types/MageSquadTypes.h"
#include "MSBaseProjectile.generated.h"

/**
 * 작성자: 김준형
 * 작성일: 25/12/15
 *
 */
UCLASS()
class MAGESQUAD_API AMSBaseProjectile : public AActor
{
	GENERATED_BODY()

public:
	AMSBaseProjectile();

	const UProjectileStaticData* GetProjectileStaticData() const;

	// 발사체 데이터
	UPROPERTY(BlueprintReadOnly, Replicated)
	TSubclassOf<UProjectileStaticData> ProjectileDataClass;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 발사체가 정지했을 경우 콜백 함수
	UFUNCTION()
	void OnProjectileStop(const FHitResult& ImpactResult);

private:
	// 발사체 메쉬
	UPROPERTY()
	TObjectPtr<class UStaticMeshComponent> ProjectileMesh = nullptr;

	// 발사체 무브먼트 컴포넌트
	UPROPERTY()
	TObjectPtr<class UProjectileMovementComponent> ProjectileMovementComponent = nullptr;
};
