// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/AI/BTTask/BTTask_ActivateRandomAbility.h"

#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "Enemy/MSBaseEnemy.h"

UBTTask_ActivateRandomAbility::UBTTask_ActivateRandomAbility()
{
	NodeName = "Activate Random Ability";
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_ActivateRandomAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	APawn* ControlledPawn = AIController->GetPawn();
	IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(ControlledPawn);
	if (!ASCInterface)
	{
		return EBTNodeResult::Failed;
	}

	UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent();
	if (!ASC || AbilityTagPool.IsEmpty())
	{
		return EBTNodeResult::Failed;
	}

	// 랜덤 태그 선택
	TArray<FGameplayTag> Tags;
	AbilityTagPool.GetGameplayTagArray(Tags);
	const FGameplayTag& SelectedTag = Tags[FMath::RandRange(0, Tags.Num() - 1)];

	// 메모리 초기화
	FBTActivateAbilityMemory* Memory = CastInstanceNodeMemory<FBTActivateAbilityMemory>(NodeMemory);
	Memory->ASC = ASC;

	// 어빌리티 종료 델리게이트 바인딩
	Memory->EndedDelegateHandle = ASC->OnAbilityEnded.AddUObject(
		this,
		&UBTTask_ActivateRandomAbility::OnAbilityEnded,
		&OwnerComp,
		NodeMemory
	);

	// 어빌리티 활성화 시도
	FGameplayTagContainer TagContainer(SelectedTag);
	TArray<FGameplayAbilitySpec*> MatchingSpecs;
	ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(TagContainer, MatchingSpecs);

	if (MatchingSpecs.Num() > 0)
	{
		if (ASC->TryActivateAbility(MatchingSpecs[0]->Handle))
		{
			Memory->ActivatedAbilityHandle = MatchingSpecs[0]->Handle;
			return EBTNodeResult::InProgress;
		}
	}

	// 실패 시 델리게이트 해제
	ASC->OnAbilityEnded.Remove(Memory->EndedDelegateHandle);
	return EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTTask_ActivateRandomAbility::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTActivateAbilityMemory* Memory = CastInstanceNodeMemory<FBTActivateAbilityMemory>(NodeMemory);

	if (Memory->ASC.IsValid())
	{
		// 델리게이트 해제
		Memory->ASC->OnAbilityEnded.Remove(Memory->EndedDelegateHandle);
		
		// 어빌리티 캔슬
		Memory->ASC->CancelAbilityHandle(Memory->ActivatedAbilityHandle);
	}

	return EBTNodeResult::Aborted;
}

uint16 UBTTask_ActivateRandomAbility::GetInstanceMemorySize() const
{
	return sizeof(FBTActivateAbilityMemory);
}

FString UBTTask_ActivateRandomAbility::GetStaticDescription() const
{
	return FString::Printf(TEXT("Random Ability from %d tags"), AbilityTagPool.Num());
}

void UBTTask_ActivateRandomAbility::OnAbilityEnded(const FAbilityEndedData& EndedData, UBehaviorTreeComponent* OwnerComp, uint8* NodeMemory) const
{
	FBTActivateAbilityMemory* Memory = CastInstanceNodeMemory<FBTActivateAbilityMemory>(NodeMemory);
	
	// 우리가 활성화한 어빌리티인지 확인
	if (EndedData.AbilitySpecHandle != Memory->ActivatedAbilityHandle)
	{
		return;
	}

	// 델리게이트 해제
	if (Memory->ASC.IsValid())
	{
		Memory->ASC->OnAbilityEnded.Remove(Memory->EndedDelegateHandle);
	}

	// Task 완료
	FinishLatentTask(*OwnerComp, EndedData.bWasCancelled ? EBTNodeResult::Failed : EBTNodeResult::Succeeded);
}
