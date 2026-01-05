// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MSGameMode.generated.h"

/*
* 작성자: 이상준
* 작성일: 2025-12-20
* 인게임 전용 GameMode
* - 사용할 GameFlow 클래스 결정
* - 전체 게임 제한 시간 설정
* - 게임 시작 가능 여부 판단 (모든 플레이어 준비 완료)
* 
* 작성자: 임희섭
* 작성일: 2025-12-20
* - 게임 시작시 몬스터 스폰너를 통해 몬스터 스폰
*/
UCLASS()
class MAGESQUAD_API AMSGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	AMSGameMode();
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void RestartPlayer(AController* NewPlayer) override;

public:
	// GameState에서 GameFlow 생성 시 참조용
	TSubclassOf<class UMSGameFlowBase> GetGameFlowClass() const;
	float GetTotalGameTime() const { return TotalGameTime; }
	// 모든 플레이어가 UI가 준비되어있으면 게임시작
	void TryStartGame();
	void NotifyClientsShowLoadingWidget();
private:
	void SetupGameFlow();
	void OnMissionFinished(int32 MissionId, bool bSuccess);
	void ExecuteTravelToLobby();
protected:
	//미션 타임라인 DataTable
	UPROPERTY(EditDefaultsOnly, Category = "GameFlow")
	class UDataTable* MissionTimelineTable;
	// 사용할 GameFlow 클래스 (PvE / PvP 등)
	UPROPERTY(EditDefaultsOnly, Category = "GameFlow")
	TSubclassOf<class UMSGameFlowBase> GameFlowClass;
	UPROPERTY()
	class UMSGameFlowBase* GameFlow = nullptr;
	UPROPERTY(EditDefaultsOnly, Category = "GameFlow")
	float TotalGameTime = 600.0f;

};
