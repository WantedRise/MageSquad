// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Types/GameFlowTypes.h"
#include "MSGameFlowBase.generated.h"

/*
* 작성자: 이상준
* 작성일: 2025-12-16
*
* 인게임 진행 흐름(Game Flow)을 관리하는 추상 베이스 클래스.
*
* - 게임의 현재 진행 상태(EGameFlowState)를 보관
* - GameState를 소유자로 하여 서버 기준으로 흐름을 제어
* - 모드(PvE / PvP / Event)에 따라 상속 클래스로 교체 가능
*
*/
UCLASS()
class MAGESQUAD_API UMSGameFlowBase : public UObject
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(class AMSGameState* OwnerGS);
	virtual void TickFlow(float DeltaSeconds);

	EGameFlowState GetGameFlowState() const { return GameFlowState; }
protected:
	//상태 전환용 내부 함수
	void SetState(EGameFlowState NewState);

protected:
	UPROPERTY()
	EGameFlowState GameFlowState = EGameFlowState::None;
	UPROPERTY()
	class AMSGameState* OwnerGameState = nullptr;
};
