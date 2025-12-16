// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/AIController/MSBaseAIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

class UCharacterMovementComponent;

AMSBaseAIController::AMSBaseAIController()
{
	// MoveTo 시 회전 방식 설정
	bAllowStrafe = false;  // 무조건 회전해서 이동
}

void AMSBaseAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	// 클라이언트에서는 AI 로직 실행 안함
	if (GetNetMode() == NM_Client)
	{
		return;
	}
	
	// RVO 설정 확인 (Pawn의 Movement Component)
	// if (ACharacter* OwnerCharacter = Cast<ACharacter>(InPawn))
	// {
	// 	UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement();
	// 	if (MoveComp && !MoveComp->bUseRVOAvoidance)
	// 	{
	// 		UE_LOG(LogTemp, Warning, TEXT("%s: RVO Avoidance is not enabled!"),   
	// 			   *InPawn->GetName());
	// 	}
	// }
	
	RunAI();
}

void AMSBaseAIController::RunAI()
{
	// 블랙보드 컴포넌트 받아오기.
	UBlackboardComponent* BB = Blackboard.Get();
	
	// BehaviorTree에 연결된 BlackboardAsset 사용
	if (BehaviorTreeAsset && BehaviorTreeAsset->BlackboardAsset)
	{
		if (UseBlackboard(BehaviorTreeAsset->BlackboardAsset, BB))
		{
			RunBehaviorTree(BehaviorTreeAsset);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: BehaviorTree or BlackboardAsset is null!"), *GetName());
	}
}

void AMSBaseAIController::StopAI()
{
	// 실행 중인 BT 컴포넌트 받아서 중단
	if (UBehaviorTreeComponent* BT = Cast<UBehaviorTreeComponent>(BrainComponent))
	{
		BT->StopTree();
	}
}