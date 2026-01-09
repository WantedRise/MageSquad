// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Tasks/MSAT_EnemyMove.h"

#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Enemy/AIController/MSBaseAIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

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
		// 초기 이동 방향 설정
		CurrentMoveDirection = OwnerActor->GetActorForwardVector();
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
	
	if (!Ability->GetActorInfo().IsNetAuthority())
	{
		EndTask();
		return;
	}
	
	// 서버에서만 실행됨 (Activate에서 클라이언트는 이미 종료)
	if (!CachedAIC.IsValid())
	{
		OnTargetLost.Broadcast();
		EndTask();
		return;
	}
	
	// Freeze 상태면 이동 업데이트 스킵
	APawn* OwnerPawn = CachedAIC->GetPawn();
	if (OwnerPawn)
	{
		const UCharacterMovementComponent* CMC = Cast<ACharacter>(OwnerPawn)->GetCharacterMovement();
		if (CMC && CMC->MaxWalkSpeed <= 0.f)
		{
			// 진행 중인 이동도 중단
			CachedAIC->StopMovement();
			return;  // 이동 업데이트 스킵
		}
	}
	
	const UBlackboardComponent* BB = CachedAIC->GetBlackboardComponent();
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

	if (!OwnerPawn)
	{
		OnTargetLost.Broadcast();
		EndTask();
		return;
	}
	
	// 주기적으로 경로 업데이트
	PathUpdateTimer += DeltaTime;
	if (PathUpdateTimer >= PathUpdateInterval || CachedPathPoints.Num() == 0)
	{
		PathUpdateTimer = 0.f;
		UpdateCachedPath(OwnerPawn, TargetActor);
	}

	// 경로 방향 가져와서 관성 적용
	FVector DesiredDirection = GetNextPathDirection(OwnerPawn, TargetActor);
	
	// UE_LOG(LogTemp, Warning, TEXT("[Move] Before - CurrentDir: %s, DesiredDir: %s"), 
	// *CurrentMoveDirection.ToString(), 
	// *DesiredDirection.ToString());
    
	CurrentMoveDirection = FMath::VInterpTo(
		CurrentMoveDirection,
		DesiredDirection,
		DeltaTime,
		TurnSpeed
	);
	
	// UE_LOG(LogTemp, Warning, TEXT("[Move] After - CurrentDir: %s"), 
	//   *CurrentMoveDirection.ToString());

	ACharacter* OwnerChar = Cast<ACharacter>(OwnerPawn);
	OwnerChar->AddMovementInput(CurrentMoveDirection, 1.f);

	// TimeSinceLastUpdate += DeltaTime;
	// if (TimeSinceLastUpdate >= UpdateInterval)
	// {
	// 	TimeSinceLastUpdate = 0.f;
	// 	UpdateMoveTarget();
	// }
}

void UMSAT_EnemyMove::OnDestroy(bool bInOwnerFinished)
{
	if (CachedAIC.IsValid())
	{
		CachedAIC->StopMovement();
	}
	Super::OnDestroy(bInOwnerFinished);
}

// void UMSAT_EnemyMove::UpdateMoveTarget()
// {
// 	const UBlackboardComponent* BB = CachedAIC->GetBlackboardComponent();
// 	if (!BB)
// 	{
// 		OnTargetLost.Broadcast();
// 		EndTask();
// 		return;
// 	}
//
// 	AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(CachedAIC->GetTargetActorKey()));
//     
// 	if (!TargetActor)
// 	{
// 		OnTargetLost.Broadcast();
// 		EndTask();
// 		return;
// 	}
//
// 	APawn* OwnerPawn = CachedAIC->GetPawn();
// 	if (!OwnerPawn)
// 	{
// 		OnTargetLost.Broadcast();
// 		EndTask();
// 		return;
// 	}
//
// 	const FVector CurrentLocation = OwnerPawn->GetActorLocation();
// 	const FVector TargetLocation = TargetActor->GetActorLocation();
// 	const float DistToTargetSq = FVector::DistSquared(CurrentLocation, TargetLocation);
//
// 	// 도달 체크
// 	if (DistToTargetSq <= FMath::Square(AcceptanceRadius))
// 	{
// 		//CachedAIC->StopMovement();
// 		OnTargetReached.Broadcast();
// 		//EndTask();
// 		return;
// 	}
//
// 	// 타겟 위치가 충분히 변경되었을 때만 경로 재계산
// 	const float TargetMovedDistSq = FVector::DistSquared(LastTargetLocation, TargetLocation);
// 	if (TargetMovedDistSq > TargetLocationThresholdSq || LastTargetLocation.IsZero())
// 	{
// 		LastTargetLocation = TargetLocation;
// 		CachedAIC->MoveToActor(TargetActor, AcceptanceRadius);
// 	}
// }

FVector UMSAT_EnemyMove::GetNextPathDirection(const APawn* OwnerPawn, const AActor* TargetActor)
{
	
	UE_LOG(LogTemp, Warning, TEXT("[Path] PathPoints: %d, CurrentIndex: %d"), 
		CachedPathPoints.Num(), CurrentPathIndex);
    
	if (CachedPathPoints.Num() > 0 && CurrentPathIndex < CachedPathPoints.Num())
	{
		float DistToPoint = FVector::Dist2D(
			OwnerPawn->GetActorLocation(), 
			CachedPathPoints[CurrentPathIndex]
		);
		UE_LOG(LogTemp, Warning, TEXT("[Path] DistToCurrentPoint: %f"), DistToPoint);
	}
	
	if (CachedPathPoints.Num() <= 1)
	{
		// 경로가 없거나 시작점만 있으면 직선 방향
		return (TargetActor->GetActorLocation() - OwnerPawn->GetActorLocation()).GetSafeNormal2D();
	}
    
	// 시작점(인덱스 0)은 스킵, 1부터 시작
	if (CurrentPathIndex == 0)
	{
		CurrentPathIndex = 1;
	}
    
	// 현재 경로 지점에 도달했으면 다음 지점으로
	while (CurrentPathIndex < CachedPathPoints.Num())
	{
		float DistToPoint = FVector::DistSquared2D(
			OwnerPawn->GetActorLocation(), 
			CachedPathPoints[CurrentPathIndex]
		);
        
		if (DistToPoint < FMath::Square(100.f))  // 100 유닛 이내면 도달
		{
			CurrentPathIndex++;
		}
		else
		{
			break;
		}
	}
    
	if (CurrentPathIndex < CachedPathPoints.Num())
	{
		return (CachedPathPoints[CurrentPathIndex] - OwnerPawn->GetActorLocation()).GetSafeNormal2D();
	}
    
	return (TargetActor->GetActorLocation() - OwnerPawn->GetActorLocation()).GetSafeNormal2D();
}

void UMSAT_EnemyMove::UpdateCachedPath(const APawn* OwnerPawn, const AActor* TargetActor)
{
	CachedPathPoints.Empty();
	CurrentPathIndex = 0;
    
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys) return;
    
	FVector StartLoc = OwnerPawn->GetActorLocation();
	FVector EndLoc = TargetActor->GetActorLocation();
    
	UNavigationPath* NavPath = NavSys->FindPathToLocationSynchronously(
		GetWorld(),
		StartLoc,
		EndLoc,
		const_cast<APawn*>(OwnerPawn)  // Querier로 Pawn 전달
	);
    
	if (NavPath && NavPath->IsValid())
	{
		for (const FVector& Point : NavPath->PathPoints)
		{
			CachedPathPoints.Add(Point);
            
			// 디버그
			// DrawDebugSphere(GetWorld(), Point, 30.f, 8, FColor::Yellow, false, PathUpdateInterval);
		}
		UE_LOG(LogTemp, Warning, TEXT("[Path] Generated %d points"), CachedPathPoints.Num());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Path] FindPathToLocation failed!"));
	}
}
