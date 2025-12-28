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
	virtual void SetPoolingMode(const bool bInPooling) override;
	virtual void SetPhase2SkeletalMesh(USkeletalMesh* NewSkeletalMesh) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_TransitionToPhase2();
	
	
public:
	UFUNCTION()
	void OnRep_Phase2SkeletalMesh(USkeletalMesh* NewSkeletalMesh);
	
public:
	FORCEINLINE UAnimMontage* GetSpawnMontage() const {return Cast<UDA_EnemyBossAnimationSet>(AnimData)->SpawnAnim;}
	FORCEINLINE UAnimMontage* GetGroggyMontage() const {return Cast<UDA_EnemyBossAnimationSet>(AnimData)->GroggyAnim;}
	FORCEINLINE USkeletalMesh* GetPhase2SkeletalMesh() const {return Phase2SkeletalMesh;}
	
private:
	UPROPERTY(ReplicatedUsing= OnRep_Phase2SkeletalMesh, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMesh> Phase2SkeletalMesh;
};
