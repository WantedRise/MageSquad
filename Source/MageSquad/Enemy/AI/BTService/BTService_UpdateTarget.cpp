// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/AI/BTService/BTService_UpdateTarget.h"
#include "MSGameplayTags.h"
#include "AbilitySystem/AttributeSets/MSEnemyAttributeSet.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Enemy/MSBaseEnemy.h"
#include "Enemy/AIController/MSBaseAIController.h"
#include "Enemy/AIController/MSBossAIController.h"
#include "GameFramework/PlayerState.h"
#include "GameStates/MSGameState.h"
#include "Player/MSPlayerCharacter.h"

UBTService_UpdateTarget::UBTService_UpdateTarget()
{
	NodeName = TEXT("UpdateTarget");
	
	Interval = 0.5f;
	
}

void UBTService_UpdateTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	AMSBaseAIController* AIController = Cast<AMSBaseAIController>(OwnerComp.GetAIOwner());
	if (!AIController)
	{
		return;
	}
	
	if (OwnerComp.GetBlackboardComponent()->GetValueAsBool(AIController->GetIsDeadKey()))
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsBool(AIController->GetCanAttackKey(), false);
		return;
	}
	
	if (AMSBossAIController* BossAIController = Cast<AMSBossAIController>(AIController))
	{
		if (OwnerComp.GetBlackboardComponent()->GetValueAsBool(BossAIController->GetIsGroggyKey()))
		{
			OwnerComp.GetBlackboardComponent()->SetValueAsBool(AIController->GetCanAttackKey(), false);
			return;
		}
	}
	
	AMSBaseEnemy* OwnerPawn = Cast<AMSBaseEnemy>(AIController->GetPawn());
	if (!OwnerPawn)
	{
		return;
	}
	
	AMSGameState* GameState = GetWorld()->GetGameState<AMSGameState>();
	if (!GameState)
	{
		return;
	}
	
	AActor* CurrentTarget = nullptr;
	float ClosestDistanceSq = FLT_MAX;
	FVector MyLocation = OwnerPawn->GetActorLocation();
	
	for (APlayerState* PS : GameState->PlayerArray)
	{
		if (!PS)
		{
			continue;
		}
		
		// PlayerState가 소유한 Pawn(플레이어 캐릭터) 가져오기
		AMSPlayerCharacter* PlayerCharacter = Cast<AMSPlayerCharacter>(PS->GetPawn());
		if (!PlayerCharacter)
		{
			continue;
		}
		
		if (UAbilitySystemComponent* ASC = PlayerCharacter->GetAbilitySystemComponent())
		{
			if (ASC->HasMatchingGameplayTag(MSGameplayTags::Player_State_Dead))
			{
				continue;
			}
		}

		// 거리 계산 및 최단 거리 갱신
		float DistanceSq = FVector::DistSquared(MyLocation, PlayerCharacter->GetActorLocation());
		if (DistanceSq < ClosestDistanceSq)
		{
			ClosestDistanceSq = DistanceSq;
			CurrentTarget = PlayerCharacter;
		}
	}
	
	// 4. 유효한 타겟이 없는 경우 처리
	if (!CurrentTarget)
	{
		// Idle 상태라면 유지 (기존 로직)
		if (OwnerPawn->GetAbilitySystemComponent()->HasMatchingGameplayTag(MSGameplayTags::Enemy_State_Idle))
		{
			return;
		}
		
		OwnerComp.GetBlackboardComponent()->ClearValue(AIController->GetTargetActorKey());
		OwnerComp.GetBlackboardComponent()->ClearValue(AIController->GetTargetLocationKey());
		OwnerComp.GetBlackboardComponent()->SetValueAsBool(AIController->GetCanAttackKey(), false);
		return;
	}
	
	// 5. 공격 범위 체크 및 블랙보드 업데이트
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
