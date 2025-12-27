// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/AI/BTService/BTService_UpdateTarget.h"
#include "EngineUtils.h"
#include "MSGameplayTags.h"
#include "AbilitySystem/AttributeSets/MSEnemyAttributeSet.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Enemy/MSBaseEnemy.h"
#include "Enemy/AIController/MSBaseAIController.h"
#include "Player/MSPlayerCharacter.h"

UBTService_UpdateTarget::UBTService_UpdateTarget()
{
	NodeName = TEXT("UpdateTarget");
	
	Interval = 0.5f;
	
}

void UBTService_UpdateTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	// 타겟 검색 및 갱신
	if (AMSBaseAIController* AIController = Cast<AMSBaseAIController>(OwnerComp.GetAIOwner()))
	{
			
		if (OwnerComp.GetBlackboardComponent()->GetValueAsBool(AIController->GetIsDeadKey()))
		{
			OwnerComp.GetBlackboardComponent()->SetValueAsBool(AIController->GetCanAttackKey(), false);
			return;
		}
			
		// @Todo : 나중에 그냥 플레이 리스트 저장하도록 수정할 예정
		TArray<AMSPlayerCharacter*> FoundActors;
		
		// 월드에 있는 Player 액터를 검색해서 배열에 추가
		for (AMSPlayerCharacter* Player : TActorRange<AMSPlayerCharacter>(GetWorld()))
		{
			FoundActors.Add(Player);	
		}
		
		AMSBaseEnemy* OwnerPawn = Cast<AMSBaseEnemy>(OwnerComp.GetAIOwner()->GetPawn());
		if (!OwnerPawn)
		{
			return;
		}
		
		// 가장 가까운 플레이어 찾기
		AActor* CurrentTarget = nullptr;
		float ClosestDistanceSq = FLT_MAX;
		FVector MyLocation = OwnerPawn->GetActorLocation();

		for (AMSPlayerCharacter* Player : FoundActors)
		{
			if (UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent())
			{
				if (ASC->HasMatchingGameplayTag(MSGameplayTags::Player_State_Dead))
				{
					continue;
				}
				
				float DistanceSq = FVector::DistSquared(MyLocation, Player->GetActorLocation());
				if (DistanceSq < ClosestDistanceSq)
				{
					ClosestDistanceSq = DistanceSq;
					CurrentTarget = Player;
				}
			}
		}
		
		// 유효한 타겟이 없는 경우
		if (!CurrentTarget)
		{
			if (OwnerPawn->GetAbilitySystemComponent()->HasMatchingGameplayTag(MSGameplayTags::Enemy_State_Idle))
			{
				return;
			}
			
			OwnerComp.GetBlackboardComponent()->ClearValue(AIController->GetTargetActorKey());
			OwnerComp.GetBlackboardComponent()->ClearValue(AIController->GetTargetLocationKey());
			OwnerComp.GetBlackboardComponent()->SetValueAsBool(AIController->GetCanAttackKey(), false);
			return;
		}
		
		// AttributeSet에서 Range 가져오기
		const UMSEnemyAttributeSet* AttributeSet = Cast<const UMSEnemyAttributeSet>(OwnerPawn->GetAbilitySystemComponent()->GetAttributeSet(
			UMSEnemyAttributeSet::StaticClass()));
    
		if (!AttributeSet)
		{
			return;
		}
	
		float AttackRangeSq = (AttributeSet->GetAttackRange() * AttributeSet->GetAttackRange());
		OwnerComp.GetBlackboardComponent()->SetValueAsBool(AIController->GetCanAttackKey(), ClosestDistanceSq <= AttackRangeSq);
		
		OwnerComp.GetBlackboardComponent()->SetValueAsObject(
			AIController->GetTargetActorKey(), CurrentTarget);
		
		OwnerComp.GetBlackboardComponent()->SetValueAsVector(
			AIController->GetTargetLocationKey(), CurrentTarget->GetActorLocation());
	}
}
