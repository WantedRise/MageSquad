// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MSGameMode.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API AMSGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;

public:
	// GameState에서 GameFlow 생성 시 참조용
	TSubclassOf<class UMSGameFlowBase> GetGameFlowClass() const;
	float GetTotalGameTime() const { return TotalGameTime; }
	// 모든 플레이어가 UI가 준비되어있으면 게임시작
	void TryStartGame();
protected:
	UPROPERTY(EditDefaultsOnly, Category = "GameFlow")
	TSubclassOf<class UMSGameFlowBase> GameFlowClass;
	UPROPERTY(EditDefaultsOnly, Category = "GameFlow")
	float TotalGameTime = 600.0f;
};
