// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/AI/BTDecorator/BTDecorator_CanAttack.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_CanAttack::UBTDecorator_CanAttack()
{
	NodeName = TEXT("Check Can Attack");
	
	// Blackboard Key Selector가 Bool 타입 키만 선택할 수 있도록 필터를 추가
	CanAttackKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_CanAttack, CanAttackKey));
}

bool UBTDecorator_CanAttack::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return false; // Blackboard가 없으면 조건 실패
	}

	// Blackboard Key의 이름을 가져와 그 값을 Bool로 읽음
	const FName KeyName = CanAttackKey.SelectedKeyName;
	
	bool bCanAttack = BlackboardComp->GetValueAsBool(KeyName);
	
	return bCanAttack;
}