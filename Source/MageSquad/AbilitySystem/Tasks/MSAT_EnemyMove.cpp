// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Tasks/MSAT_EnemyMove.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Enemy/AIController/MSBaseAIController.h"

UMSAT_EnemyMove* UMSAT_EnemyMove::CreateTask(UGameplayAbility* OwningAbility, float AcceptanceRadius,
                                             float TargetUpdateInterval)
{
	UMSAT_EnemyMove* Task = NewAbilityTask<UMSAT_EnemyMove>(OwningAbility);
	Task->AcceptanceRadius = AcceptanceRadius;
	Task->UpdateInterval = TargetUpdateInterval;
	Task->TimeSinceLastUpdate = TargetUpdateInterval; // 즉시 첫 이동
	Task->bTickingTask = true;
	Task->LastTargetLocation = FVector::ZeroVector;
	Task->TargetLocationThresholdSq = FMath::Square(100.f); // 타겟이 100 이상 움직여야 경로 재계산
	return Task;
}

void UMSAT_EnemyMove::Activate()
{
	Super::Activate();
	
	// 서버 전용 체크 - 클라이언트에서는 즉시 종료
	if (!Ability->GetActorInfo().IsNetAuthority())
	{
		EndTask();
		return;
	}

	if (AActor* OwnerActor = Ability->GetActorInfo().OwnerActor.Get())
	{
		CachedAIC = Cast<AMSBaseAIController>(OwnerActor->GetInstigatorController());
	}

	if (!CachedAIC.IsValid())
	{
		OnTargetLost.Broadcast();
		EndTask();
	}
}

void UMSAT_EnemyMove::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);
	
	// 서버에서만 실행됨 (Activate에서 클라이언트는 이미 종료)
	if (!CachedAIC.IsValid())
	{
		OnTargetLost.Broadcast();
		EndTask();
		return;
	}

	TimeSinceLastUpdate += DeltaTime;
	if (TimeSinceLastUpdate >= UpdateInterval)
	{
		TimeSinceLastUpdate = 0.f;
		UpdateMoveTarget();
	}
}

void UMSAT_EnemyMove::OnDestroy(bool bInOwnerFinished)
{
	if (CachedAIC.IsValid())
	{
		CachedAIC->StopMovement();
	}
	Super::OnDestroy(bInOwnerFinished);
}

void UMSAT_EnemyMove::UpdateMoveTarget()
{
	UBlackboardComponent* BB = CachedAIC->GetBlackboardComponent();
	if (!BB)
	{
		OnTargetLost.Broadcast();
		EndTask();
		return;
	}

	AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(CachedAIC->GetTargetActorKey()));
    
	if (!TargetActor)
	{
		OnTargetLost.Broadcast();
		EndTask();
		return;
	}

	APawn* OwnerPawn = CachedAIC->GetPawn();
	if (!OwnerPawn)
	{
		OnTargetLost.Broadcast();
		EndTask();
		return;
	}

	const FVector CurrentLocation = OwnerPawn->GetActorLocation();
	const FVector TargetLocation = TargetActor->GetActorLocation();
	const float DistToTargetSq = FVector::DistSquared(CurrentLocation, TargetLocation);

	// 도달 체크
	if (DistToTargetSq <= FMath::Square(AcceptanceRadius))
	{
		CachedAIC->StopMovement();
		OnTargetReached.Broadcast();
		EndTask();
		return;
	}

	// 타겟 위치가 충분히 변경되었을 때만 경로 재계산
	const float TargetMovedDistSq = FVector::DistSquared(LastTargetLocation, TargetLocation);
	if (TargetMovedDistSq > TargetLocationThresholdSq || LastTargetLocation.IsZero())
	{
		LastTargetLocation = TargetLocation;
		CachedAIC->MoveToActor(TargetActor, AcceptanceRadius);
	}
}
