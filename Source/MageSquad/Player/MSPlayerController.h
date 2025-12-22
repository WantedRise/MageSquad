// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DataStructs/MSGameMissionData.h"
#include "MSPlayerController.generated.h"


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

	UFUNCTION()
	void HandleMissionChanged(int32 MissionID);

	// 미션이 시작된 시점에 미션 UI 타이밍 설정
	void HandleMissionStarted(const FMSMissionRow& MissionData);

	void ShowMissionTitle(FMSMissionRow MissionData);

	// 미션 진행 UI 보이기
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
};
