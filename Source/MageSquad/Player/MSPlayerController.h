// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DataStructs/MSGameMissionData.h"
#include "Types/MageSquadTypes.h"
#include "InputActionValue.h"
#include "MSPlayerController.generated.h"

struct FMSMissionProgressUIData;
class UMSLevelUpPanel;
/**
 * 작성자: 김준형
 * 작성일: 25/12/08
 *
 * 플레이어 컨트롤러 클래스
 * 로컬 HUD 생성/표시 (네트워크 동기화는 HUD 위젯 내부에서 처리)
 * 각 컨트롤러의 커서 위치를 서버에게 알림
 */

 /**
  * 작성자: 이상준
  * 작성일: 25/12/23
  *
  * 미션 시작 / 진행 / 종료에 따른 UI 연출 흐름 제어
  * GameState의 미션 이벤트에 미션 UI 바인딩
  * 맵 로딩을 위한 딜레이, 로딩창을 2초뒤 제거
  */
UCLASS()
class MAGESQUAD_API AMSPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void SetPawn(APawn* InPawn) override;
	virtual void OnRep_Pawn() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override; // 시그니피컨스 매니저 사용을 위한 추가 - 26/01/05 임희섭


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
	// 미션 진행도 변경 시 호출되는 콜백
	UFUNCTION()
	void OnMissionProgressChanged(const FMSMissionProgressUIData& Data);

	// 미션 UI의 표시 타이밍과 초기 설정을 처리
	void HandleMissionStarted(const FMSMissionRow& MissionData);

	// 미션 제목 및 시작 연출 UI를 출력
	void ShowMissionTitle(FMSMissionRow MissionData);

	// 현재 미션의 진행 상황을 추적하는 UI를 표시
	void ShowMissionTracker(FMSMissionRow MissionData);
public:
	// 로딩 표시
	UFUNCTION(Client, Reliable)
	void ClientShowLoadingWidget();
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

	// UI 준비 완료 서버에 보고
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
	* Spectator Section
	*****************************************************/
public:
	// 서버: 관전 대상 설정 함수
	void SetSpectateTarget_Server(AActor* NewTarget);

	// 관전 대상 변경 함수
	void SetSpectateViewTarget(AActor* NewTarget);

	// 사망 상태에 따른 관전 입력 적용 함수
	void ApplyLocalInputState(bool bDead);

	// 보스 컷씬 시작/종료 상태 설정 (관전 카메라 전환과 충돌 방지)
	void SetBossCutsceneActive(bool bActive);

	// 현재 복귀해야 할 카메라 타깃 반환
	AActor* GetDesiredViewTarget() const;

	// 컷씬 종료 후 보류된 카메라 타깃 적용
	void ApplyPendingViewTarget();

	// 서버: 현재 관전 대상이 유효하지 않으면 자동으로 다음 대상으로 전환하는 함수
	void EnsureValidSpectateTarget_Server();

	// 현재 관전 대상 Getter
	UFUNCTION(BlueprintCallable, Category = "Custom | Spectate")
	AActor* GetSpectateTargetActor() const { return SpectateTargetActor; }

protected:
	// 생존 중인 팀원 탐색 후 관전 사이클을 돌리는 함수
	void CycleSpectateTarget(int32 Direction);

	// 관전 대상 변경 OnRep 함수. 로컬 클라이언트에서만 카메라 전환 수행
	UFUNCTION()
	void OnRep_SpectateTargetActor();

private:
	// 관전 대상 변경 입력 콜백 함수
	void OnSpectatePrevAction(const FInputActionValue& Value);
	void OnSpectateNextAction(const FInputActionValue& Value);

	// 관전 대상 변경 요청 함수
	void RequestChangeSpectate(int32 Direction);

	// 서버: 관전 대상 변경 함수
	void HandleChangeSpectate_Server(int32 Direction);

	// 서버에게 관전 대상 변경을 요청하는 함수 ServerRPC
	UFUNCTION(Server, Reliable)
	void ServerRPCChangeSpectate(int32 Direction);

protected:
	/* ================== Spectate Camera Blend Option ================== */
	// 관전 카메라 전환 블랜딩 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Spectate | Camera")
	float SpectateBlendTime = 0.5f;

	// 관전 카메라 전환 블랜딩 함수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Spectate | Camera")
	TEnumAsByte<EViewTargetBlendFunction> SpectateBlendFunction = VTBlend_Cubic;
	/* ================== Spectate Camera Blend Option ================== */


	/* ================== Spectate Input Option ================== */
	// 기본 게임 플레이 IMC (플레이어 캐릭터의 기본 IMC)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Spectate | Input")
	TObjectPtr<class UInputMappingContext> DefaultIMC;

	// 관전 전용 IMC
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Spectate | Input")
	TObjectPtr<class UInputMappingContext> SpectateIMC;

	// 관전 대상 변경(이전) 입력 액션
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Spectate | Input")
	TObjectPtr<class UInputAction> SpectatePrevAction;

	// 관전 대상 변경(다음) 입력 액션
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Spectate | Input")
	TObjectPtr<class UInputAction> SpectateNextAction;

private:
	// 현재 관전 대상
	// - 서버가 이 값을 갱신하면, 소유 클라이언트에서 OnRep로 카메라 전환을 수행
	UPROPERTY(ReplicatedUsing = OnRep_SpectateTargetActor)
	TObjectPtr<AActor> SpectateTargetActor = nullptr;

	// 컷씬 중 ViewTarget 변경을 보류하기 위한 캐시
	TWeakObjectPtr<AActor> PendingViewTarget;

	// 보스 컷씬 진행 여부
	bool bBossCutsceneActive = false;



	/*****************************************************
	* Skill Level Up Section
	*****************************************************/
public:
	/** 서버 -> 클라: 레벨업 선택지 UI 띄우기 */
	UFUNCTION(Client, Reliable)
	void Client_ShowSkillLevelUpChoices(int32 SessionId, const TArray<FMSLevelUpChoicePair>& Choices, float RemainingSeconds);

	UFUNCTION(Client, Reliable)
	void Client_CloseSkillLevelUpChoices(int32 SessionId, bool bKeepPaused);

	/** 클라 -> 서버: 선택 결과 전달 (Panel에서 호출) */
	UFUNCTION(Server, Reliable)
	void Server_SelectSkillLevelUpChoice(int32 SessionId, const FMSLevelUpChoicePair& Picked);

protected:
	/** BP에서 WBP_LevelUpPanel 지정 */
	UPROPERTY(EditDefaultsOnly, Category = "UI|LevelUp")
	TSubclassOf<UMSLevelUpPanel> LevelUpPanelClass;

public:
	/** 현재 떠있는 패널 인스턴스 */
	UPROPERTY(Transient)
	TObjectPtr<UMSLevelUpPanel> LevelUpPanelInstance = nullptr;


	/*****************************************************
	* End Game Section
	*****************************************************/
public:
	// 게임 종료 위젯 표시 함수
	UFUNCTION(Client, Reliable)
	void ClientRPCShowEndGameWidget(TSubclassOf<class UUserWidget> WidgetClass, int32 ZOrder = 500);

protected:
	// 게임 종료 위젯 인스턴스
	UPROPERTY(Transient)
	TObjectPtr<class UUserWidget> EndGameWidget;
};
