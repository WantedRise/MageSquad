// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/AI/BTDecorator/BTDecorator_CheckHasTag.h"

#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "Enemy/MSBaseEnemy.h"

UBTDecorator_CheckHasTag::UBTDecorator_CheckHasTag()
{
	NodeName = TEXT("Check Has Tag");
}

bool UBTDecorator_CheckHasTag::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	bool Result = Super::CalculateRawConditionValue(OwnerComp, NodeMemory);
	// Owner Pawn 가져오기
	AMSBaseEnemy* OwnerPawn = Cast<AMSBaseEnemy>(OwnerComp.GetAIOwner()->GetPawn());
	if (!OwnerPawn)
	{
		Result = false;
		return Result;
	}
	
	
	for (const auto& Tag : GameplayTags)
	{
		Result = OwnerPawn->GetAbilitySystemComponent()->HasMatchingGameplayTag(Tag);
		if (Result == false)
		{
			break;
		}
	}
	
	return Result;
}
