// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Types/MageSquadTypes.h"
#include "Components/SphereComponent.h"
#include "MSBaseProjectile.generated.h"

/**
 * 발사체 베이스 액터. 런타임 데이터와 행동(Behavior)을 소유한다.
 */
class UStaticMeshComponent;
class UProjectileMovementComponent;
class USceneComponent;
class UMSProjectileBehaviorBase;

UCLASS()
class MAGESQUAD_API AMSBaseProjectile : public AActor
{
	GENERATED_BODY()

public:
	AMSBaseProjectile();

	// StaticData 클래스로부터 RuntimeData를 초기화.
	void InitProjectileRuntimeDataFromClass(TSubclassOf<UProjectileStaticData> InProjectileDataClass);

	// 런타임 데이터 접근자.
	const FProjectileRuntimeData& GetProjectileRuntimeData() const { return ProjectileRuntimeData; }
	FProjectileRuntimeData GetEffectiveRuntimeData() const;
	void SetProjectileRuntimeData(const FProjectileRuntimeData& InRuntimeData);

	// 콜리전 관련.
	void SetCollisionRadius(float Radius);
	void EnableCollision(bool bEnable);
	void AddIgnoredActor(AActor* Actor);
	bool IsIgnoredActor(const AActor* Actor) const;

	// RuntimeData.SFX 배열 인덱스로 SFX 재생.
	void PlaySFXAtLocation(int32 Index) const;
	void PlaySFXAttached(int32 Index, USceneComponent* AttachTo) const;

	// 이동 관련.
	void StopMovement();
	UProjectileMovementComponent* GetMovementComponent() const
	{
		return ProjectileMovementComponent;
	}

	// 부착 VFX 1회 재생.
	void SpawnAttachVFXOnce();

	// 폭발 연출: 정지 및 숨김 (멀티캐스트).
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopAndHide(const FVector& InLocation);


protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnProjectileStop(const FHitResult& ImpactResult);

	UFUNCTION()
	void OnHitOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnRep_ProjectileRuntimeData();

protected:
	// Behavior 생성/초기화 및 RuntimeData 반영.
	void EnsureBehavior();
	void ApplyProjectileRuntimeData(bool bSpawnAttachVFX);
	void ArmLifeTimerIfNeeded(const FProjectileRuntimeData& EffectiveData);

protected:
	// 런타임 데이터 복제.
	UPROPERTY(ReplicatedUsing = OnRep_ProjectileRuntimeData)
	FProjectileRuntimeData ProjectileRuntimeData;

	UPROPERTY(Replicated)
	bool bRuntimeDataInitialized = false;

	// 콜리전 및 비주얼.
	UPROPERTY()
	TObjectPtr<USphereComponent> CollisionSphere = nullptr;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> ProjectileMesh = nullptr;

	UPROPERTY()
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent = nullptr;

	// 현재 행동 인스턴스.
	UPROPERTY(Transient)
	TObjectPtr<UMSProjectileBehaviorBase> Behavior = nullptr;

	// 수명 타이머.
	FTimerHandle LifeTimerHandle;

	// 부착 VFX 1회 재생용 플래그.
	bool bAttachVfxSpawned = false;

	// 중복 히트 방지용 ignore 목록.
	TSet<TWeakObjectPtr<AActor>> IgnoredActors;

public:
	// 발사체 StaticData 클래스.
	UPROPERTY(Replicated)
	TSubclassOf<UProjectileStaticData> ProjectileDataClass;
};
