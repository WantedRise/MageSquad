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
class UNiagaraSystem;
class AGameStateBase;

USTRUCT()
struct FSplitProjectileEvent
{
	GENERATED_BODY()

	UPROPERTY()
	bool bValid = false;

	UPROPERTY()
	FVector_NetQuantize Origin = FVector::ZeroVector;

	UPROPERTY()
	FVector_NetQuantizeNormal DirA = FVector::ForwardVector;

	UPROPERTY()
	FVector_NetQuantizeNormal DirB = FVector::ForwardVector;

	UPROPERTY()
	uint8 NumDirs = 0;

	UPROPERTY()
	int32 PenetrationCount = 0;
};

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
	void AddSimPathPoint(const FVector& Point);
	void ClearSimPathPoints();
	const TArray<FVector_NetQuantize>& GetSimPathPoints() const { return SimPathPoints; }

	// RuntimeData.SFX 배열 인덱스로 SFX 재생.
	void PlaySFXAtLocation(int32 Index);
	void PlaySFXAttached(int32 Index, USceneComponent* AttachTo);

	// 파괴 요청 (중복 호출 방지).
	void RequestDestroy();

	// 클라 시뮬 파라미터 접근자.
	bool IsClientSimEnabled() const { return bClientSimEnabled; }
	FVector GetClientSimStartLocation() const { return SimStartLocation; }
	FVector GetClientSimDirection() const { return SimDirection; }
	float GetClientSimSpeed() const { return SimSpeed; }
	float GetClientSimStartTime() const;
	FVector GetClientSimCorrectionOffset(float DeltaSeconds, const FVector& SimulatedLocation);
	int32 GetClientSimNoiseSeed() const { return SimNoiseSeed; }

	// 분열 연출 이벤트(클라이언트 전용 스폰).
	void TriggerSplitEvent(const FVector& Origin, const FVector& DirA, const FVector& DirB, int32 PenetrationCount);

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

	// SFX 재생 멀티캐스트.
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlaySFXAtLocation(int32 Index, const FVector& Location);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlaySFXAttached(int32 Index);

	// VFX 재생 멀티캐스트.
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_SpawnVFXAtLocation(UNiagaraSystem* Vfx, const FVector& Location, float Scale);

	// 클라 전용 투사체 스폰(서버는 스킵).
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpawnClientProjectiles(
		TSubclassOf<UProjectileStaticData> DataClass,
		const FProjectileRuntimeData& BaseData,
		const FVector& Origin,
		const TArray<FVector_NetQuantizeNormal>& Directions
	);

	// ChainBolt 클라 이동 단계 전달.
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_ChainBoltStep(
		const FVector& Start,
		const FVector& Target,
		float Speed,
		float Interval,
		int32 StepId
	);

	// 서버 판정 종료 신호.
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ServerStop(const FVector& InLocation);


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

	UFUNCTION()
	void OnRep_SplitEvent();

	UFUNCTION()
	void OnRep_SimServerLocation();

	UFUNCTION()
	void OnRep_ClientSimEnabled();

protected:
	// Behavior 생성/초기화 및 RuntimeData 반영.
	void EnsureBehavior();
	void ApplyProjectileRuntimeData(bool bSpawnAttachVFX);
	void ArmLifeTimerIfNeeded(const FProjectileRuntimeData& EffectiveData);
	void UpdateSimServerLocation();

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
	FTimerHandle SimCorrectionTimerHandle;

	bool bDestroyRequested = false;

	UPROPERTY(ReplicatedUsing = OnRep_SplitEvent)
	FSplitProjectileEvent SplitEvent;

	UPROPERTY(Replicated)
	int32 SplitEventId = 0;

	UPROPERTY(ReplicatedUsing = OnRep_ServerStop)
	bool bServerStop = false;

	UFUNCTION()
	void OnRep_ServerStop();

	int32 LastHandledSplitEventId = 0;

	// 클라 시뮬 파라미터 (서버가 설정, 클라가 재현).
	UPROPERTY(ReplicatedUsing = OnRep_ClientSimEnabled)
	bool bClientSimEnabled = false;

	UPROPERTY(Replicated)
	float SimStartServerTime = 0.f;

	UPROPERTY(Replicated)
	FVector_NetQuantize SimStartLocation = FVector::ZeroVector;

	UPROPERTY(Replicated)
	FVector_NetQuantizeNormal SimDirection = FVector::ForwardVector;

	UPROPERTY(Replicated)
	float SimSpeed = 0.f;

	UPROPERTY(Replicated)
	int32 SimNoiseSeed = 0;

	UPROPERTY(Replicated)
	TArray<FVector_NetQuantize> SimPathPoints;

	UPROPERTY(ReplicatedUsing = OnRep_SimServerLocation)
	FVector_NetQuantize SimServerLocation = FVector::ZeroVector;

	FVector SimCorrectionStart = FVector::ZeroVector;
	FVector SimCorrectionTarget = FVector::ZeroVector;
	float SimCorrectionAlpha = 1.f;
	float SimCorrectionDuration = 0.5f;
	bool bHasSimCorrection = false;

	// 부착 VFX 1회 재생용 플래그.
	bool bAttachVfxSpawned = false;

	// 중복 히트 방지용 ignore 목록.
	TSet<TWeakObjectPtr<AActor>> IgnoredActors;

public:
	// 발사체 StaticData 클래스.
	UPROPERTY(Replicated)
	TSubclassOf<UProjectileStaticData> ProjectileDataClass;
};
