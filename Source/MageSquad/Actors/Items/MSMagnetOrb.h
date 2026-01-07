// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Items/MSItemOrb.h"
#include "MSMagnetOrb.generated.h"

/**
 * 작성자: 김준형
 * 작성일: 26/01/07
 *
 * 자석 오브 액터
 * 오브 획득 시, 해당 플레이어가 현재 레벨에 존재하는 모든 경험치 오브를 획득
 */
UCLASS()
class MAGESQUAD_API AMSMagnetOrb : public AMSItemOrb
{
	GENERATED_BODY()

public:
	AMSMagnetOrb();

	/*
	* 서버: 오브 획득 처리 함수
	* 반드시 서버에서만 호출되어야 하며, 자식 클래스는 Super::Collect_Server를 호출한 후, 각자 효과를 구현
	* @param CollectorActor: 획득자
	*/
	virtual void Collect_Server(AActor* CollectorActor) override;
};
