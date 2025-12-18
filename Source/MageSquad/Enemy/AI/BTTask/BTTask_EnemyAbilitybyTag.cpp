// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/AI/BTTask/BTTask_EnemyAbilitybyTag.h"
#include "AbilitySystemComponent.h"
#include "Enemy/MSBaseEnemy.h"

UBTTask_EnemyAbilitybyTag::UBTTask_EnemyAbilitybyTag()
{
}

EBTNodeResult::Type UBTTask_EnemyAbilitybyTag::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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
