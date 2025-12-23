// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DataStructs/MSGameMissionData.h"
#include "Types/MageSquadTypes.h"
#include "MSPlayerController.generated.h"


class UMSLevelUpPanel;
/**
 * 작성자: 김준형
 * 작성일: 25/12/08
 *
 * 플레이어 컨트롤러 클래스
 * 로컬 HUD 생성/표시 (네트워크 동기화는 HUD 위젯 내부에서 처리)
 * 각 컨트롤러의 커서 위치를 서버에게 알림
 */
UCLASS()
class MAGESQUAD_API AMSPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnRep_Pawn() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;



	/*****************************************************
	* HUD Section
	*****************************************************/
public:
	// HUD 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | UI")
	TSubclassOf<class UMSPlayerHUDWidget> HUDWidgetClass;

protected:
	// HUD 생성 함수. (로컬에서만 HUD를 생성/표시)
	void EnsureHUDCreated();

	// Pawn이 준비된 시점에 HUD 바인딩(재시도 포함) 요청
	void NotifyHUDReinitialize();
private: /* 미션 */
	// 새로운 미션으로 전환되었을 때 호출되는 콜백 함수
	UFUNCTION()
	void OnMissionChanged(int32 MissionID);
	// 현재 진행 중인 미션이 종료(성공/실패)되었을 때 호출되는 콜백 함수
	UFUNCTION()
	void OnMissionFinished(int32 MissionId, bool bSuccess);

	// 미션 UI의 표시 타이밍과 초기 설정을 처리
	void HandleMissionStarted(const FMSMissionRow& MissionData);

	// 미션 제목 및 시작 연출 UI를 출력
	void ShowMissionTitle(FMSMissionRow MissionData);

	// 현재 미션의 진행 상황을 추적하는 UI를 표시
	void ShowMissionTracker(FMSMissionRow MissionData);
protected:
	// HUD 위젯 인스턴스
	UPROPERTY(Transient)
	TObjectPtr<class UMSPlayerHUDWidget> HUDWidgetInstance;



	/*****************************************************
	* Mouse Curosr Section
	*****************************************************/
public:
	// 서버에서 읽는 최신 커서 위치
	FVector GetServerCursor() const;

	// 서버에서 읽는 최신 커서 방향
	FVector GetServerCursorDir(const FVector& FallbackForward) const;

private:
	// 마우스 커서 업데이트 함수
	void UpdateCursor();

	// 커서 정보 업데이트 RPC
	UFUNCTION(Server, Unreliable)
	void ServerRPCSetCursorInfo(const FVector_NetQuantize& InPos, const FVector_NetQuantizeNormal& InDir);

	UFUNCTION(Server, Reliable)
	void ServerRPCReportReady();
private:
	// 마우스 커서 업데이트 타이머
	FTimerHandle CursorUpdateTimer;

	// 마우스 커서 위치
	UPROPERTY()
	FVector_NetQuantize ServerCursor = FVector(0.f, 0.f, 0.f);

	// 마우스 커서 방향
	UPROPERTY()
	FVector_NetQuantize ServerCursorDir = FVector(1.f, 0.f, 0.f);
	
	UPROPERTY(BlueprintReadOnly, Category = "MVVM", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UMSMVVM_PlayerViewModel> PlayerViewModel;
	
	/*****************************************************
	* Skill Level Up Section
	*****************************************************/
public:
	/** 서버 -> 클라: 레벨업 선택지 UI 띄우기 */
	UFUNCTION(Client, Reliable)
	void Client_ShowSkillLevelUpChoices(int32 SessionId, const TArray<FMSLevelUpChoicePair>& Choices);

	/** 클라 -> 서버: 선택 결과 전달 (Panel에서 호출) */
	UFUNCTION(Server, Reliable)
	void Server_SelectSkillLevelUpChoice(int32 SessionId, const FMSLevelUpChoicePair& Picked);
	
protected:
	/** BP에서 WBP_LevelUpPanel 지정 */
	UPROPERTY(EditDefaultsOnly, Category="UI|LevelUp")
	TSubclassOf<UMSLevelUpPanel> LevelUpPanelClass;
	
private:
	/** 현재 떠있는 패널 인스턴스 */
	UPROPERTY(Transient)
	TObjectPtr<UMSLevelUpPanel> LevelUpPanelInstance = nullptr;
};
