// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "DataAssets/Enemy/DA_MonsterAnimationSetData.h"
#include "GameFramework/Character.h"
#include "SignificanceManager.h"
#include "MSBaseEnemy.generated.h"

/*
 * 작성자 : 임희섭
 * 작성일 : 2025/12/12
 * 몬스터들의 Base 클래스
 * 몬스터들의 공통 특성을 정의한다. 
 */

struct FMSEnemyStaticData;

UCLASS()
class MAGESQUAD_API AMSBaseEnemy : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMSBaseEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void PostInitializeComponents() override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// ~ Begin IAbilitySystemInterface Interface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	class UMSEnemyAttributeSet* GetAttributeSet() { return AttributeSet; }
	
	// ~ End IAbilitySystemInterface Interface

	// ~ Begin IMSHitReactableInterface Interface
	//virtual void OnHitByAttack_Implementation(const FHitResult& HitResult, AActor* InInstigator) override;
	// ~ End IMSHitReactableInterface Interface

	UFUNCTION()
	virtual void OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	                                   const FHitResult& SweepResult);

	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget,
	                              const FVector& SrcLocation) const override;

public:
	void SetMonsterID(const FName& NewMonsterID);
	void SetAnimData(UDA_EnemyAnimationSet* NewAnimData);
	virtual void SetPhase2SkeletalMesh(USkeletalMesh* NewSkeletalMesh) {};

	// 풀링 모드 제어
	virtual void SetPoolingMode(bool bInPooling);

public: /*Getter*/
	FORCEINLINE UAnimMontage* GetAttackMontage() const { return AnimData->AttackAnim; }
	FORCEINLINE UAnimMontage* GetDeadMontage() const { return AnimData->DeadAnim; }
	FORCEINLINE UAnimMontage* GetMoveMontage() const { return AnimData->WalkAnim; }
	FORCEINLINE UAnimMontage* GetIdleMontage() const { return AnimData->IdleAnim; }	
	FORCEINLINE TSubclassOf<class UGameplayEffect> GetDamageEffectClass() const { return DamageEffectClass; }
	FORCEINLINE TSubclassOf<class UGameplayEffect> GetCooldownEffectClass() const { return CooldownEffectClass; }
	FORCEINLINE TSubclassOf<class UProjectileStaticData> GetProjectileDataClass() const { return ProjectileDataClass; }

public: /*Setter*/
	FORCEINLINE void SetProjectileData(TSubclassOf<class UProjectileStaticData> InProjectileDataClass)
	{
		ProjectileDataClass = InProjectileDataClass;
	}

protected:
	UFUNCTION()
	virtual void OnRep_MonsterID();
	
	UFUNCTION()
	void OnRep_IsInPool();
	
public:
	// 시그니컨스 매니저가 호출할 계산 함수
	static float CalculateSignificance(USignificanceManager::FManagedObjectInfo* ObjectInfo, const FTransform& Viewpoint);
	
	// 중요도가 변경되었을 때 호출될 커스텀 함수
	// 매개변수: 정보객체, 이전 중요도, 새로운 중요도, 화면에 보이는지 여부
	void OnSignificanceChanged(USignificanceManager::FManagedObjectInfo* ObjectInfo, float OldSig, float NewSig, bool bInView);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UMSEnemyAbilitySystemComponent> ASC;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UMSEnemyAttributeSet> AttributeSet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<class UGameplayAbility>> StartAbilities;

	UPROPERTY(ReplicatedUsing = OnRep_MonsterID)
	FName CurrentMonsterID = "";

	UPROPERTY()
	TObjectPtr<class UDA_EnemyAnimationSet> AnimData;

	UPROPERTY(EditDefaultsOnly, Category = "GAS|Attack")
	TSubclassOf<class UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "GAS|Cooldown")
	TSubclassOf<class UGameplayEffect> CooldownEffectClass;
	
	UPROPERTY(ReplicatedUsing = OnRep_IsInPool)
	bool bIsInPool = false;
	
	UPROPERTY()
	uint8 bCanOptimization = true;

private:
	// 발사체 원본 데이터 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UProjectileStaticData> ProjectileDataClass;
};
