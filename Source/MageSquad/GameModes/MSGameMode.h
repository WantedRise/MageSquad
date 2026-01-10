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

	virtual void Tick(float DeltaTime) override;

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


	/*****************************************************
	* End Game Section
	*****************************************************/
protected:
	// 게임 승리 or 패배 위젯을 모든 플레이어에게 표시하는 함수
	void ShowEndGameWidgetToAllPlayers(bool bIsVictory);

	// 로비 이동 콜백 함수
	void TravelToLobby_Internal();

protected:
	// 게임 승리 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | UI")
	TSubclassOf<class UUserWidget> GameVictoryWidgetClass;

	// 게임 패배 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | UI")
	TSubclassOf<class UUserWidget> GameOverWidgetClass;

	// 로비로 이동하기 전 대기 시간(게임 종료 위젯 표시 시간) (초)
	UPROPERTY(EditDefaultsOnly, Category = "Custom | UI")
	float TravelDelaySeconds = 3.85f;

private:
	// 로비 이동 카운트다운
	float LobbyTravelCountdown = -1.f;

	// 중복 호출 방지 플래그
	bool bTravelScheduled = false;

	// 플레이어 전원 사망 플래그 (승리/패배 판단용)
	bool bAllPlayersDead = false;
};
