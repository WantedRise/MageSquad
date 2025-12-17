// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "MSGameState.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API AMSGameState : public AGameState
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;
public:
	//실행 요청(스폰은 여기서 직접 하지 말고, 델리게이트/GameMode로 넘겨도 OK)
	void RequestSpawnFinalBoss();
	//보스 처치 여부(또는 다른 종료 조건)
	bool IsFinalBossDefeated() const;
	//인게임 전체 시간
	float GetGameElapsedTime() const { return GameElapsedTime; }

	class UMSGameFlowBase* GetGameFlow() const { return GameFlow; }
protected:
	//UPROPERTY(Replicated)
	float GameElapsedTime;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "GameFlow")
	class UMSGameFlowBase* GameFlow;
};
