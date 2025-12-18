// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/AI/BTTask/BTTask_NormalAttack.h"

#include "AbilitySystemComponent.h"
#include "Enemy/MSBaseEnemy.h"

UBTTask_NormalAttack::UBTTask_NormalAttack()
{
	NodeName = TEXT("Normal Attack");
}

EBTNodeResult::Type UBTTask_NormalAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type result = Super::ExecuteTask(OwnerComp, NodeMemory);
	
	if (GameplayTags.Num() == 0)
	{
		result = EBTNodeResult::Failed;
		return result;
	}
	
	if (UAbilitySystemComponent* ASC = OwnerEnemy->GetAbilitySystemComponent())
	{
		ASC->TryActivateAbilitiesByTag(GameplayTags);
	}
	
	return result;
}
