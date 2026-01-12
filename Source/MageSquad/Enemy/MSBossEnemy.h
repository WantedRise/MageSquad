// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataAssets/Enemy/DA_EnemyBossAnimationSet.h"
#include "Enemy/MSBaseEnemy.h"
#include "Interfaces/MSIndicatorDamageInterface.h"
#include "MSBossEnemy.generated.h"

/**
 * 작성자 : 임희섭
 * 작성일 : 2025/12/12
 * 보스를 나타내는 클래스
 */
UCLASS()
class MAGESQUAD_API AMSBossEnemy : public AMSBaseEnemy, public IMSIndicatorDamageInterface
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
	void Multicast_TransitionToPhase2();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlaySpawnCutscene(bool bStart);

public:
	UFUNCTION()
	void OnRep_Phase2SkeletalMesh(USkeletalMesh* NewSkeletalMesh);

public: /*Getter*/
	FORCEINLINE UAnimMontage* GetSpawnMontage() const { return Cast<UDA_EnemyBossAnimationSet>(AnimData)->SpawnAnim; }
	FORCEINLINE UAnimMontage* GetGroggyMontage() const { return Cast<UDA_EnemyBossAnimationSet>(AnimData)->GroggyAnim; }
	FORCEINLINE UAnimMontage* GetPattern1Montage() const
	{
		return Cast<UDA_EnemyBossAnimationSet>(AnimData)->Partten1Anim;
	}

	FORCEINLINE UAnimMontage* GetPattern2Montage() const
	{
		return Cast<UDA_EnemyBossAnimationSet>(AnimData)->Partten2Anim;
	}

	FORCEINLINE UAnimMontage* GetPattern3Montage() const
	{
		return Cast<UDA_EnemyBossAnimationSet>(AnimData)->Partten3Anim;
	}
	FORCEINLINE class UMSDirectionIndicatorComponent* GetDirectionIndicatorComponent() const
	{
		return DirectionIndicatorComponent;
	}

	FORCEINLINE USkeletalMesh* GetPhase2SkeletalMesh() const { return Phase2SkeletalMesh; }
	FORCEINLINE class UCameraComponent* GetBossCamera() const { return Camera; }
	FORCEINLINE class USpringArmComponent* GetBossSpringArm() const {return SpringArm; }

protected:
	virtual UAbilitySystemComponent* GetIndicatorSourceASC_Implementation() const override;
	virtual TSubclassOf<UGameplayEffect> GetIndicatorDamageEffect_Implementation() const override;

private:
	void TrySetMesh(USkeletalMesh* NewSkeletalMesh);

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

	// 방향 표시 인디케이터 컴포넌트 (팀원에게 위치 및 초상화 제공)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom | Indicator", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UMSDirectionIndicatorComponent> DirectionIndicatorComponent;

	UPROPERTY()
	TSubclassOf<class UTexture2D> IndicatorImage;
};
