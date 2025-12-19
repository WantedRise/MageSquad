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
* - 게임모드 블루프린트에서 선택하여 모드(PvE / PvP)에 따라 게임규칙 정해짐
*
*/
UCLASS(Blueprintable)
class MAGESQUAD_API UMSGameFlowBase : public UObject
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(class AMSGameState* InOwnerGameState, float InTotalGameTime);
	virtual void TickFlow(float DeltaSeconds);
	virtual void Start();
	EGameFlowState GetCurrentState() const { return CurrentState; }
protected:
	// ProgressComponent 이벤트 수신
	virtual void OnGameTimeReached(float ElapsedSeconds);
	//상태 전환용 내부 함수
	void SetState(EGameFlowState NewState);
	virtual void OnEnterState(EGameFlowState NewState);
	virtual void OnExitState(EGameFlowState PreState);

	class AMSGameState* GetOwnerGameState() const { return OwnerGameState; }
protected:
	UPROPERTY(VisibleAnywhere, Category = "GameFlow")
	EGameFlowState CurrentState = EGameFlowState::None;
	UPROPERTY()
	class AMSGameState* OwnerGameState = nullptr;
	UPROPERTY()
	class UMSGameProgressComponent* GameProgress = nullptr;
};
