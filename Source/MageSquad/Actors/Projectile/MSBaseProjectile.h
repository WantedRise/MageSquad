// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Types/MageSquadTypes.h"
#include "Components/SphereComponent.h"
#include "MSBaseProjectile.generated.h"

/**
 * ?‘ì„±?? ê¹€ì¤€??
 * ?‘ì„±?? 25/12/15
 *
 * ?˜ì •: ë°•ì„¸ì°?
 * ë°œì‚¬ì²? ê³µê²©??ëª¸ì²´ ??• 
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

	// ?ë³¸ StaticDataë¡œë???RuntimeData ì´ˆê¸°??
	void InitProjectileRuntimeDataFromClass(TSubclassOf<UProjectileStaticData> InProjectileDataClass);

	// ?°í????°ì´??Getter
	const FProjectileRuntimeData& GetProjectileRuntimeData() const { return ProjectileRuntimeData; }

	// ? íš¨???°í????°ì´??ë°˜í™˜
	FProjectileRuntimeData GetEffectiveRuntimeData() const;

	// ?°í????°ì´??Setter (GAê°€ ë§Œë“  RuntimeData ì£¼ì…)
	void SetProjectileRuntimeData(const FProjectileRuntimeData& InRuntimeData);

	// Collision ë°˜ê²½ ?¤ì • (?¥íŒ/?¬ì‚¬ì²?ê³µìš©)
	void SetCollisionRadius(float Radius);

	// Collision on/off
	void EnableCollision(bool bEnable);
	// Ignore hit for specific actor
	void AddIgnoredActor(AActor* Actor);
	bool IsIgnoredActor(const AActor* Actor) const;

	// ?´ë™ ?•ì? (?¥íŒ??
	void StopMovement();

	// ProjectileMovement ?‘ê·¼ (Behavior?ì„œ ì§ì ‘ ?¸íŒ…)
	UProjectileMovementComponent* GetMovementComponent() const
	{
		return ProjectileMovementComponent;
	}

	// Attach VFX (??ë²ˆë§Œ)
	void SpawnAttachVFXOnce();

	// Explosive¿ë ½Ã°¢ ¿¬Ãâ µ¿±âÈ­
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopAndHide(const FVector& InLocation);
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ë²?ë°”ë‹¥ ??Block?¼ë¡œ ë©ˆì·„????
	UFUNCTION()
	void OnProjectileStop(const FHitResult& ImpactResult);

	// ?ê³¼ ì¶©ëŒ ??
	UFUNCTION()
	void OnHitOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	// ?°í????°ì´??ë³µì œ ???´ë¼) ?ìš©
	UFUNCTION()
	void OnRep_ProjectileRuntimeData();



protected:
	// ?œë²„/?´ë¼ ëª¨ë‘?ì„œ Behavior ë³´ì¥
	void EnsureBehavior();

	// ?°í????°ì´???ìš©(ë©”ì‹œ/?¤ì???ë¬´ë¸Œë¨¼íŠ¸/VFX/?˜ëª… ?€?´ë¨¸)
	void ApplyProjectileRuntimeData(bool bSpawnAttachVFX);

	// LifeTime ?€?´ë¨¸ ?¸íŒ… (?œë²„)
	void ArmLifeTimerIfNeeded(const FProjectileRuntimeData& EffectiveData);
	
protected:
	// ìµœì¢… RuntimeData
	UPROPERTY(ReplicatedUsing = OnRep_ProjectileRuntimeData)
	FProjectileRuntimeData ProjectileRuntimeData;

	// RuntimeDataê°€ ??ë²ˆì´?¼ë„ ì´ˆê¸°?”ë˜?ˆëŠ”ì§€
	UPROPERTY(Replicated)
	bool bRuntimeDataInitialized = false;

	// ?ˆíŠ¸ ?ì •??ì½œë¦¬????Overlap ?„ìš©)
	UPROPERTY()
	TObjectPtr<USphereComponent> CollisionSphere = nullptr;

	// ?œê°/ë¸”ë¡œ?¹ìš© ë©”ì‹œ
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> ProjectileMesh = nullptr;

	// ?´ë™ ì²˜ë¦¬
	UPROPERTY()
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent = nullptr;

	// ?‰ë™(?¬ì‚¬ì²??¥íŒ/ì§€?? ê°ì²´
	UPROPERTY(Transient)
	TObjectPtr<UMSProjectileBehaviorBase> Behavior = nullptr;

	// ?˜ëª… ?€?´ë¨¸(ë¡œì»¬ ë³€?˜ë¡œ ?ë©´ ì¤‘ë³µ ë²„ê·¸ ?ê?)
	FTimerHandle LifeTimerHandle;

	// Attach VFX ì¤‘ë³µ ?ì„± ë°©ì? (ë¡œì»¬)
	bool bAttachVfxSpawned = false;

	// Overlap ignore list
	TSet<TWeakObjectPtr<AActor>> IgnoredActors;

public:
	// ?ë³¸ StaticData ?´ë˜??
	UPROPERTY(Replicated)
	TSubclassOf<UProjectileStaticData> ProjectileDataClass;
};


