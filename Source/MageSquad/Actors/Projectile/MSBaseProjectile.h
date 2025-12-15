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

	// 발사체 원본 데이터를 복제하여 발사체 재정의 데이터를 초기화하는 함수
	void InitProjectileRuntimeDataFromClass(TSubclassOf<UProjectileStaticData> InProjectileDataClass);

	// 발사체 런타임 데이터 Getter
	const FProjectileRuntimeData& GetProjectileRuntimeData() const { return ProjectileRuntimeData; }

	// 발사체 런타임 데이터 Setter
	void SetProjectileRuntimeData(const FProjectileRuntimeData& InRuntimeData);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 발사체가 정지했을 경우 콜백 함수
	UFUNCTION()
	void OnProjectileStop(const FHitResult& ImpactResult);

private:
	// 런타임 데이터 적용 함수
	void ApplyProjectileRuntimeData(bool bSpawnAttachVFX);

	// 발사체 런타임 데이터 네트워크 복제 함수
	UFUNCTION()
	void OnRep_ProjectileRuntimeData();

public:
	// 발사체 원본 데이터
	UPROPERTY(Replicated)
	TSubclassOf<UProjectileStaticData> ProjectileDataClass;

	// 발사체 재정의 데이터
	UPROPERTY(ReplicatedUsing = OnRep_ProjectileRuntimeData)
	FProjectileRuntimeData ProjectileRuntimeData;

private:
	// 발사체 메쉬
	UPROPERTY()
	TObjectPtr<class UStaticMeshComponent> ProjectileMesh = nullptr;

	// 발사체 무브먼트 컴포넌트
	UPROPERTY()
	TObjectPtr<class UProjectileMovementComponent> ProjectileMovementComponent = nullptr;

	// 런타임 데이터의 초기화 여부(원본 데이터에서 복제된 런타임 데이터가 있는지)
	UPROPERTY(Replicated)
	bool bRuntimeDataInitialized = false;
};
