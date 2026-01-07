// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Items/MSItemOrb.h"
#include "MSExperienceOrb.generated.h"

/**
 * 작성자: 김준형
 * 작성일: 25/12/21
 * 수정일: 26/01/07 (아이템 오브 추상화)
 *
 * 경험치 오브 액터
 * 오브 획득 시, 공유 경험치 획득
 */
UCLASS()
class MAGESQUAD_API AMSExperienceOrb : public AMSItemOrb
{
	GENERATED_BODY()

public:
	AMSExperienceOrb();

	/*
	* 서버: 오브 획득 처리 함수
	* 반드시 서버에서만 호출되어야 하며, 자식 클래스는 Super::Collect_Server를 호출한 후, 각자 효과를 구현
	* @param CollectorActor: 획득자
	*/
	virtual void Collect_Server(AActor* CollectorActor) override;

public:
	// 경험치 오브가 제공하는 기본 경험치(보정 전)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Experience")
	float ExperienceValue = 10.f;
};
