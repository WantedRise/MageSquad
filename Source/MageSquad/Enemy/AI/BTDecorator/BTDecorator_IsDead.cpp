// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/AI/BTDecorator/BTDecorator_IsDead.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_IsDead::UBTDecorator_IsDead()
{
	NodeName = TEXT("Check Is Dead");
	
	// Blackboard Key Selector가 Bool 타입 키만 선택할 수 있도록 필터를 추가
	IsDeadKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_IsDead, IsDeadKey));
}

bool UBTDecorator_IsDead::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	Super::CalculateRawConditionValue(OwnerComp, NodeMemory);
	
	const UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return false; 
	}

	const FName KeyName = IsDeadKey.SelectedKeyName;
	bool bIsDead = BlackboardComp->GetValueAsBool(KeyName);
	
	return bIsDead;
}
