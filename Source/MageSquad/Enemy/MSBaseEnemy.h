// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "MSBaseEnemy.generated.h"

/*
 * 작성자 : 임희섭
 * 작성일 : 2025/12/08
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
	// ~ End IAbilitySystemInterface Interface
	
public:
	void SetMonsterID(const FName& NewMonsterID);
	
protected:
	UFUNCTION()
	void OnRep_MonsterID();
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UMSEnemyAbilitySystemComponent> ASC;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UMSEnemyAttributeSet> AttributeSet;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<class UGameplayAbility>> StartAbilities;
	
	UPROPERTY(ReplicatedUsing = OnRep_MonsterID)
	FName CurrentMonsterID;
	
};
