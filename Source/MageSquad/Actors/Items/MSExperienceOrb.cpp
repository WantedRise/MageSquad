// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Items/MSExperienceOrb.h"

#include "GameStates/MSGameState.h"

AMSExperienceOrb::AMSExperienceOrb()
{
}

void AMSExperienceOrb::Collect_Server(AActor* CollectorActor)
{
	// 서버 및 중복 획득 여부 체크를 위해 현재 상태를 저장
	const bool bWasCollected = bCollected;

	Super::Collect_Server(CollectorActor);

	// 이미 획득한 경우 더 이상 처리하지 않음
	if (!HasAuthority() || bWasCollected) return;

	// 공유 경험치 누적 함수를 호출하여 공유 경험치 획득 처리
	if (AMSGameState* GS = GetWorld() ? GetWorld()->GetGameState<AMSGameState>() : nullptr)
	{
		GS->AddSharedExperience_Server(CollectorActor, ExperienceValue);
	}
}
