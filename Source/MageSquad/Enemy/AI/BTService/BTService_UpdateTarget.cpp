// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/AI/BTService/BTService_UpdateTarget.h"
#include "EngineUtils.h"
#include "BehaviorTree/BlackboardComponent.h"
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
	
	// 매 Interval마다 실행 - 타겟 검색 및 갱신
	if (AMSBaseAIController* AIController = Cast<AMSBaseAIController>(OwnerComp.GetAIOwner()))
	{
		// 주변 적 탐지
		TArray<AMSPlayerCharacter*> FoundActors;
		// 월드에 있는 Player 액터를 검색해서 배열에 추가
		for (AMSPlayerCharacter* PlayerStart : TActorRange<AMSPlayerCharacter>(GetWorld()))
		{
			// 배열에 추가
			FoundActors.Add(PlayerStart);	
		}
		
		AActor* CurrentTarget = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(AIController->GetTargetActorKey()));
		// if (CurrentTarget != FoundActors[0])
		// {
		// Blackboard 갱신
		OwnerComp.GetBlackboardComponent()->SetValueAsObject(
			AIController->GetTargetActorKey(), FoundActors[0]);
		
		OwnerComp.GetBlackboardComponent()->SetValueAsVector(
			AIController->GetTargetLocationKey(), FoundActors[0]->GetActorLocation());
		//}
	}
}
