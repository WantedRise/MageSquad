// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Types/MageSquadTypes.h"
#include "Components/SphereComponent.h"
#include "MSBaseProjectile.generated.h"

/**
 * 작성자: 김준형
 * 작성일: 25/12/15
 *
 * 수정: 박세찬
 * 발사체, 공격의 몸체 역할
 */

class UStaticMeshComponent;
class UProjectileMovementComponent;
class UMSProjectileBehaviorBase;

UCLASS()
class MAGESQUAD_API AMSBaseProjectile : public AActor
{
	GENERATED_BODY()

public:
	AMSBaseProjectile();

	// 원본 StaticData로부터 RuntimeData 초기화
	void InitProjectileRuntimeDataFromClass(TSubclassOf<UProjectileStaticData> InProjectileDataClass);

	// 런타임 데이터 Getter
	const FProjectileRuntimeData& GetProjectileRuntimeData() const { return ProjectileRuntimeData; }

	// 유효한 런타임 데이터 반환
	FProjectileRuntimeData GetEffectiveRuntimeData() const;

	// 런타임 데이터 Setter (GA가 만든 RuntimeData 주입)
	void SetProjectileRuntimeData(const FProjectileRuntimeData& InRuntimeData);

	// Collision 반경 설정 (장판/투사체 공용)
	void SetCollisionRadius(float Radius);

	// Collision on/off
	void EnableCollision(bool bEnable);

	// 이동 정지 (장판용)
	void StopMovement();

	// ProjectileMovement 접근 (Behavior에서 직접 세팅)
	UProjectileMovementComponent* GetMovementComponent() const
	{
		return ProjectileMovementComponent;
	}

	// Attach VFX (한 번만)
	void SpawnAttachVFXOnce();
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 벽/바닥 등 Block으로 멈췄을 때
	UFUNCTION()
	void OnProjectileStop(const FHitResult& ImpactResult);

	// 적과 충돌 시
	UFUNCTION()
	void OnHitOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	// 런타임 데이터 복제 시(클라) 적용
	UFUNCTION()
	void OnRep_ProjectileRuntimeData();



private:
	// 서버/클라 모두에서 Behavior 보장
	void EnsureBehavior();

	// 런타임 데이터 적용(메시/스케일/무브먼트/VFX/수명 타이머)
	void ApplyProjectileRuntimeData(bool bSpawnAttachVFX);

	// LifeTime 타이머 세팅 (서버)
	void ArmLifeTimerIfNeeded(const FProjectileRuntimeData& EffectiveData);

	
private:
	
	// 최종 RuntimeData
	UPROPERTY(ReplicatedUsing = OnRep_ProjectileRuntimeData)
	FProjectileRuntimeData ProjectileRuntimeData;

	// RuntimeData가 한 번이라도 초기화되었는지
	UPROPERTY(Replicated)
	bool bRuntimeDataInitialized = false;

	// 히트 판정용 콜리전(적 Overlap 전용)
	UPROPERTY()
	TObjectPtr<USphereComponent> CollisionSphere = nullptr;

	// 시각/블로킹용 메시
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> ProjectileMesh = nullptr;

	// 이동 처리
	UPROPERTY()
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent = nullptr;

	// 행동(투사체/장판/지속) 객체
	UPROPERTY(Transient)
	TObjectPtr<UMSProjectileBehaviorBase> Behavior = nullptr;

	// 수명 타이머(로컬 변수로 두면 중복 버그 생김)
	FTimerHandle LifeTimerHandle;

	// Attach VFX 중복 생성 방지 (로컬)
	bool bAttachVfxSpawned = false;

public:
	// 원본 StaticData 클래스
	UPROPERTY(Replicated)
	TSubclassOf<UProjectileStaticData> ProjectileDataClass;
};