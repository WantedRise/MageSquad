// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/AI/BTDecorator/BTDecorator_CanAttack.h"

#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "AbilitySystem/AttributeSets/MSEnemyAttributeSet.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Enemy/MSBaseEnemy.h"

UBTDecorator_CanAttack::UBTDecorator_CanAttack()
{
	NodeName = TEXT("Check Can Attack");
	
	// Blackboard Key Selector가 Bool 타입 키만 선택할 수 있도록 필터를 추가
	//CanAttackKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_CanAttack, CanAttackKey));
	
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_CanAttack, TargetActorKey), AActor::StaticClass());
	TargetDistanceKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_CanAttack, TargetDistanceKey));
	
	FlowAbortMode = EBTFlowAbortMode::Both;
}

bool UBTDecorator_CanAttack::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	Super::CalculateRawConditionValue(OwnerComp, NodeMemory);
	
	const UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return false;
	}

	// Target Actor 가져오기
	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!TargetActor)
	{
		return false;
	}

	// Owner Pawn 가져오기
	AMSBaseEnemy* OwnerAI = Cast<AMSBaseEnemy>(OwnerComp.GetAIOwner()->GetPawn());
	if (!OwnerAI)
	{
		return false;
	}

	// AttributeSet에서 Range 가져오기
	const UMSEnemyAttributeSet* AttributeSet = Cast<const UMSEnemyAttributeSet>(OwnerAI->GetAbilitySystemComponent()->GetAttributeSet(
		UMSEnemyAttributeSet::StaticClass()));
    
	if (!AttributeSet)
	{
		return false;
	}
	
	float AttackRange = AttributeSet->GetAttackRange();
	UE_LOG(LogTemp, Log, TEXT("AttackRange : %f"), AttackRange);
    
	float Distance = OwnerComp.GetBlackboardComponent()->GetValueAsFloat(TargetDistanceKey.SelectedKeyName);

	return Distance <= AttackRange;
}


