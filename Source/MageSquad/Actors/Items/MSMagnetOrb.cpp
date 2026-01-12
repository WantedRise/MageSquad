// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Items/MSMagnetOrb.h"

#include "Actors/Items/MSExperienceOrb.h"
#include "EngineUtils.h"

AMSMagnetOrb::AMSMagnetOrb()
{
}

void AMSMagnetOrb::Collect_Server(AActor* CollectorActor)
{
	// 서버 및 중복 획득 여부 체크를 위해 현재 상태를 저장
	const bool bWasCollected = bCollected;

	Super::Collect_Server(CollectorActor);

	// 이미 획득한 경우 더 이상 처리하지 않음
	if (!HasAuthority() || bWasCollected) return;

	UWorld* World = GetWorld();
	if (!World) return;

	// 월드 내의 모든 경험치 오브를 순회
	for (TActorIterator<AMSExperienceOrb> It(World); It; ++It)
	{
		AMSExperienceOrb* ExpOrb = *It;
		if (!IsValid(ExpOrb)) continue;

		// 경험치 오브 획득 처리 호출
		// 이미 획득된 오브는 Collect_Server 내에서 처리되므로 호출해도 무방
		ExpOrb->Collect_Server(CollectorActor);
	}
}
