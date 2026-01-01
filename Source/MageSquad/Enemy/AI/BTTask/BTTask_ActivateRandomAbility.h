// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "Enemy/AI/BTTask/BTTask_EnemyBase.h"
#include "BTTask_ActivateRandomAbility.generated.h"

struct FAbilityEndedData;
/**
 * 작성자 : 임희섭
 * 작성일 : 2026/01/01
 * Tag를 받아 Enemy들의 Ability를 시작하는 Task
 * 여러 개의 Tag를 받아 그 중 랜덤으로 골라 실행한다. 
 */

USTRUCT()
struct FBTActivateAbilityMemory
{
	GENERATED_BODY()

	TWeakObjectPtr<class UAbilitySystemComponent> ASC;
	FGameplayAbilitySpecHandle ActivatedAbilityHandle;
	FDelegateHandle EndedDelegateHandle;
};

UCLASS()
class MAGESQUAD_API UBTTask_ActivateRandomAbility : public UBTTask_EnemyBase
{
	GENERATED_BODY()
	
public:
	UBTTask_ActivateRandomAbility();
	
public:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere, Category = "Ability")
	FGameplayTagContainer AbilityTagPool;

private:
	void OnAbilityEnded(const FAbilityEndedData& EndedData, UBehaviorTreeComponent* OwnerComp, uint8* NodeMemory) const;
	
};
