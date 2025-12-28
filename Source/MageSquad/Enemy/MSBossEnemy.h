// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataAssets/Enemy/DA_EnemyBossAnimationSet.h"
#include "Enemy/MSBaseEnemy.h"
#include "MSBossEnemy.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossCutsceneStateChanged, bool, bIsStarting);

/**
 * 작성자 : 임희섭
 * 작성일 : 2025/12/12
 * 보스를 나타내는 클래스
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
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlaySpawnCutscene(bool bStart);
	
public:
	UFUNCTION()
	void OnRep_Phase2SkeletalMesh(USkeletalMesh* NewSkeletalMesh);

public:
	FORCEINLINE UAnimMontage* GetSpawnMontage() const {return Cast<UDA_EnemyBossAnimationSet>(AnimData)->SpawnAnim;}
	FORCEINLINE UAnimMontage* GetGroggyMontage() const {return Cast<UDA_EnemyBossAnimationSet>(AnimData)->GroggyAnim;}
	FORCEINLINE USkeletalMesh* GetPhase2SkeletalMesh() const {return Phase2SkeletalMesh;}
	FORCEINLINE class UCameraComponent* GetBossCamera() const { return Camera; }
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnBossCutsceneStateChanged OnBossCutsceneStateChanged;
	
private:
	UPROPERTY(ReplicatedUsing= OnRep_Phase2SkeletalMesh, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMesh> Phase2SkeletalMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom | Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom | Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> SpringArm;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom | Camera", meta = (AllowPrivateAccess = "true"))
	float TargetArmLength = 2000.f;

	UPROPERTY()
	TObjectPtr<AActor> OriginalViewTarget;
};
