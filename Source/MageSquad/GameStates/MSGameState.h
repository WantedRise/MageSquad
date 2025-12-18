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
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
	void AddElapsedGameTime(float Delta);
	void OnRep_MissionIDs();
public:
	//실행 요청(스폰은 여기서 직접 하지 말고, 델리게이트/GameMode로 넘겨도 OK)
	void RequestSpawnFinalBoss();
	//보스 처치 여부(또는 다른 종료 조건)
	bool IsFinalBossDefeated() const;
	//인게임 전체 시간
	float GetElapsedGameTime() const { return ElapsedGameTime; }
	//현재 GameFlow 반환
	class UMSGameFlowBase* GetGameFlow() const { return GameFlow; }
	//GameFlow에 시작
	void StartGame();
protected:
	//UPROPERTY(Replicated)
	//float GameStartTime;
	UPROPERTY(VisibleAnywhere)
	class UMSGameProgressComponent* GameProgress;
	UPROPERTY(Replicated)
	float ElapsedGameTime;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "GameFlow")
	class UMSGameFlowBase* GameFlow;


};
