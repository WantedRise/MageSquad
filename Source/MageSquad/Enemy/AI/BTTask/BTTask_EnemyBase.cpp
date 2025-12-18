// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/AI/BTTask/BTTask_EnemyBase.h"

#include "AIController.h"
#include "Enemy/MSBaseEnemy.h"

EBTNodeResult::Type UBTTask_EnemyBase::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);
	
	OwnerEnemy = Cast<AMSBaseEnemy>(OwnerComp.GetAIOwner()->GetPawn());
	
	return Result;
}
