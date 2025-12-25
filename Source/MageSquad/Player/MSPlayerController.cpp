// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MSPlayerController.h"
#include "GameStates/MSGameState.h"
#include "MSPlayerState.h"
#include <GameModes/MSGameMode.h>

#include "MSPlayerCharacter.h"
#include "Widgets/HUD/MSPlayerHUDWidget.h"
#include "System/MSLevelManagerSubsystem.h"
//#include "Widgets/MVVM/MSMVVM_PlayerViewModel.h"
#include "Widgets/Mission/MSMissionNotifyWidget.h"
#include "Widgets/Mission/MSMissionTrackerWidget.h"
#include "Widgets/HUD/GameProgressWidget.h"
#include "System/MSMissionDataSubsystem.h"
#include "Widgets/LevelUp/MSLevelUpPanel.h"

void AMSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 입력 모드 설정
	FInputModeGameAndUI InputMode;
	SetInputMode(InputMode);

	bShowMouseCursor = true;

	if (IsLocalController())
	{
		// 커서 트레이스 타이머 설정
		GetWorldTimerManager().SetTimer(
			CursorUpdateTimer,
			this,
			&AMSPlayerController::UpdateCursor,
			0.05f,
			true
		);

		// HUD 생성/표시
		EnsureHUDCreated();

		// BeginPlay 시점에 Pawn/ASC 준비가 끝난 경우도 있으므로 1회 재초기화 시도
		NotifyHUDReinitialize();

		// 맵 로딩을 위한 딜레이, 로딩창을 2초뒤 제거
		if (UMSLevelManagerSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UMSLevelManagerSubsystem>())
		{
			FTimerHandle MatchEntryDelayTimer;
			GetWorldTimerManager().SetTimer(
				MatchEntryDelayTimer,
				Subsystem,
				&UMSLevelManagerSubsystem::HideLoadingWidget,
				2.0f,
				false
			);
		}
		
		// ViewModel 생성
		//if (!PlayerViewModel)
		//{
		//	PlayerViewModel = NewObject<UMSMVVM_PlayerViewModel>(this);
		//}
		
		// if (UAbilitySystemComponent* ASC = Cast<AMSPlayerCharacter>(GetPawn())->GetAbilitySystemComponent())
		// {
		// 	PlayerViewModel->InitializeWithASC(ASC);
		// }
	}
}

void AMSPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// 서버/호스트에서 Pawn이 즉시 결정되는 경우가 많으므로 Possess 시점에도 HUD 재초기화
	if (IsLocalController())
	{
		EnsureHUDCreated();
		NotifyHUDReinitialize();
		ServerRPCReportReady();
	}
}

void AMSPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	// OnRep_Pawn에서도 HUD 재초기화
	// 클라이언트는 Pawn이 복제로 늦게 들어오는 경우가 많아서 HUD 재초기화를 통해 초기화 타이밍 문제를 안정화
	if (IsLocalController())
	{
		EnsureHUDCreated();
		NotifyHUDReinitialize();
		ServerRPCReportReady();
	}
}

void AMSPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 커서 트레이스 타이머 초기화
	if (CursorUpdateTimer.IsValid())
	{
		GetWorldTimerManager().ClearTimer(CursorUpdateTimer);
	}

	// HUD 위젯 제거
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->RemoveFromParent();
		HUDWidgetInstance = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void AMSPlayerController::EnsureHUDCreated()
{
	if (!IsLocalController() || HUDWidgetInstance || !HUDWidgetClass) return;

	// HUD 위젯 생성 및 그리기
	HUDWidgetInstance = CreateWidget<UMSPlayerHUDWidget>(this, HUDWidgetClass);
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->AddToViewport();
	}
}

void AMSPlayerController::NotifyHUDReinitialize()
{
	if (!IsLocalController()) return;

	if (HUDWidgetInstance)
	{
		// 위젯 내부에서 Pawn/ASC 준비 여부를 체크하고, 준비가 안 됐으면 타이머로 재시도
		HUDWidgetInstance->RequestReinitialize();
		// HUD 재초기화 시 미션 관련 이벤트 바인딩 처리
		if (AMSGameState* GS = GetWorld()->GetGameState<AMSGameState>())
		{
			if(false == GS->OnMissionChanged.IsBoundToObject(this))
				GS->OnMissionChanged.AddUObject(this, &AMSPlayerController::OnMissionChanged);
			if (false == GS->OnMissionFinished.IsBoundToObject(this))
				GS->OnMissionFinished.AddUObject(this, &AMSPlayerController::OnMissionFinished);
			if (false == GS->OnMissionProgressChanged.IsBoundToObject(this))
				GS->OnMissionProgressChanged.AddUObject(this, &AMSPlayerController::OnMissionProgressChanged);
		}	
	}
}

FVector AMSPlayerController::GetServerCursor() const
{
	return ServerCursor;
}

FVector AMSPlayerController::GetServerCursorDir(const FVector& FallbackForward) const
{
	// 화면 밖 등으로 커서 방향이 유효하지 않을 때는 캐릭터 전방으로 발사
	FVector Fwd = FVector(FallbackForward);
	Fwd.Z = 0.f;
	Fwd = Fwd.GetSafeNormal();
	if (Fwd.IsNearlyZero())
	{
		Fwd = FVector(1.f, 0.f, 0.f);
	}

	const FVector Cursor = FVector(ServerCursorDir).GetSafeNormal();

	return Cursor.IsNearlyZero() ? Fwd : Cursor;
}

void AMSPlayerController::UpdateCursor()
{
	// 커서 트레이스
	if (IsLocalController())
	{
		APawn* P = GetPawn();
		if (!P) return;

		// 마우스 커서 트레이싱
		FHitResult Hit;
		const bool bHit = GetHitResultUnderCursor(ECC_Visibility, false, Hit) && Hit.bBlockingHit;

		// 캐릭터의 위치
		const FVector SpawnOrigin = P->GetActorLocation() + FVector(0.f, 0.f, 50.f);

		// 캐릭터 전방(수평) - 커서가 화면을 벗어나면 이 방향으로 발사
		FVector Forward2D = P->GetActorForwardVector();
		Forward2D.Z = 0.f;
		Forward2D = Forward2D.GetSafeNormal();
		if (Forward2D.IsNearlyZero()) Forward2D = FVector(1.f, 0.f, 0.f);

		// 커서 월드 위치
		// - Hit 성공: 충돌 지점
		// - Hit 실패(커서가 뷰포트를 벗어남 등): 전방으로 충분히 먼 지점(가상 커서)
		const FVector CursorWorldPos = bHit ? Hit.ImpactPoint : (SpawnOrigin + Forward2D * 10000.f);

		// 커서 방향 (수평 고정)
		FVector Dir = (CursorWorldPos - SpawnOrigin);
		Dir.Z = 0.f;
		Dir = Dir.GetSafeNormal();
		if (Dir.IsNearlyZero()) Dir = Forward2D;

		if (HasAuthority())
		{
			// 호스트(리슨)면 서버 캐시 직접 갱신
			ServerCursor = CursorWorldPos;
			ServerCursorDir = Dir;
		}
		else
		{
			// 원격 클라는 서버로 전달
			ServerRPCSetCursorInfo(CursorWorldPos, Dir);
		}
	}
}

void AMSPlayerController::Client_CloseSkillLevelUpChoices_Implementation(int32 SessionId)
{
	if (!IsLocalController()) return;

	if (LevelUpPanelInstance)
	{
		LevelUpPanelInstance->RemoveFromParent();
		LevelUpPanelInstance = nullptr;
	}

	SetPause(false);

	FInputModeGameOnly Mode;
	SetInputMode(Mode);
	bShowMouseCursor = false;
}

void AMSPlayerController::Client_ShowSkillLevelUpChoices_Implementation(int32 SessionId,
                                                                        const TArray<FMSLevelUpChoicePair>& Choices, float RemainingSeconds)
{
	// 로컬 컨트롤러에서만 UI를 띄움
	if (!IsLocalController())
	{
		return;
	}

	if (!LevelUpPanelClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Client_ShowSkillLevelUpChoices] LevelUpPanelClass is NULL"));
		return;
	}

	// 이미 떠있으면 제거 후 재생성(세션 꼬임 방지)
	if (LevelUpPanelInstance)
	{
		LevelUpPanelInstance->RemoveFromParent();
		LevelUpPanelInstance = nullptr;
	}

	LevelUpPanelInstance = CreateWidget<UMSLevelUpPanel>(this, LevelUpPanelClass);
	if (!LevelUpPanelInstance)
	{
		return;
	}

	LevelUpPanelInstance->AddToViewport(200);
	LevelUpPanelInstance->InitPanel(SessionId, Choices);
	
	// 남은 시간 표시 시작
	LevelUpPanelInstance->StartCountdown(RemainingSeconds);
	
	
	SetPause(true);

	// UI 입력 모드
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(LevelUpPanelInstance->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	bShowMouseCursor = true;
}

void AMSPlayerController::Server_SelectSkillLevelUpChoice_Implementation(int32 SessionId,
	const FMSLevelUpChoicePair& Picked)
{
	AMSPlayerState* PS = GetPlayerState<AMSPlayerState>();
	if (!PS)
	{
		return;
	}
	
	PS->ApplySkillLevelUpChoice_Server(SessionId, Picked);
	
	// 완료 보고 (전원 완료 판단은 GameState)
	if (AMSGameState* GS = GetWorld()->GetGameState<AMSGameState>())
	{
		GS->NotifySkillLevelUpCompleted(PS);
	}
}

void AMSPlayerController::ServerRPCSetCursorInfo_Implementation(const FVector_NetQuantize& InPos, const FVector_NetQuantizeNormal& InDir)
{
	ServerCursor = FVector(InPos);

	// 커서 방향 저장 (수평 고정)
	FVector Dir = FVector(InDir);
	Dir.Z = 0.f;
	Dir = Dir.GetSafeNormal();
	if (Dir.IsNearlyZero())
	{
		Dir = FVector(1.f, 0.f, 0.f);
	}
	ServerCursorDir = Dir;
}
// 서버에 UI 준비 완료를 알리는 RPC
void AMSPlayerController::ServerRPCReportReady_Implementation()
{
	if (AMSPlayerState* PS = GetPlayerState<AMSPlayerState>())
	{
		PS->SetUIReady(true);
	}

	if (AMSGameMode* GM = GetWorld()->GetAuthGameMode<AMSGameMode>())
	{
		GM->TryStartGame();
	}
}

void AMSPlayerController::OnMissionChanged(int32 MissionID)
{
	if (!IsLocalController()) return;

	auto* Subsystem = GetGameInstance()->GetSubsystem<UMSMissionDataSubsystem>();
	if (const FMSMissionRow* MissionData = Subsystem ? Subsystem->Find(MissionID) : nullptr)
	{
		// 미션 시작 UI 연출 흐름 진입
		HandleMissionStarted(*MissionData);
	}
}

// 미션 시작 시 UI 연출 흐름을 관리
void AMSPlayerController::HandleMissionStarted(const FMSMissionRow& MissionData)
{
	if (!HUDWidgetInstance) return;

	auto* Notify = HUDWidgetInstance->GetMissionNotifyWidget();
	auto* Tracker = HUDWidgetInstance->GetMissionTrackerWidget();
	if (!Notify || !Tracker) return;

	ShowMissionTitle(MissionData);

	FTimerHandle TempShowMissionTrackerTimerHandle;
	GetWorldTimerManager().SetTimer(
		TempShowMissionTrackerTimerHandle,
		FTimerDelegate::CreateUObject(
			this,
			&AMSPlayerController::ShowMissionTracker,
			MissionData
		),
		2.0f,
		false
	);
}

// 미션 제목 알림 UI 출력
void AMSPlayerController::ShowMissionTitle(FMSMissionRow MissionData)
{
	auto* Notify = HUDWidgetInstance->GetMissionNotifyWidget();
	if (!Notify) return;

	Notify->PlayNotify(MissionData.Title);
}

// 미션 설명, 타이머, 진행 상태 UI를 활성화
void AMSPlayerController::ShowMissionTracker(FMSMissionRow MissionData)
{
	auto* Tracker = HUDWidgetInstance->GetMissionTrackerWidget();
	auto* Progress = HUDWidgetInstance->GetGameProgressWidget();
	if (!Tracker || !Progress) return;
	Progress->SetVisibility(ESlateVisibility::Hidden);
	Tracker->SetMissionTitle(MissionData.Title);
	Tracker->SetMissionMessage(MissionData.Description);
	if (AMSGameState* GS = GetWorld()->GetGameState<AMSGameState>())
	{
		Tracker->StartMissionTimer(GS, GS->GetMissionEndTime());
		Tracker->SetVisibility(ESlateVisibility::Visible);
	}
}

// 성공 / 실패 결과에 따른 UI 연출 수행
void AMSPlayerController::OnMissionFinished(int32 MissionID,bool bSuccess)
{
	if (!HUDWidgetInstance) return;

	auto* Notify = HUDWidgetInstance->GetMissionNotifyWidget();
	auto* Tracker = HUDWidgetInstance->GetMissionTrackerWidget();
	auto* Progress = HUDWidgetInstance->GetGameProgressWidget();
	if (!Notify || !Progress) return;

	Notify->PlayMissionResult(bSuccess);
	// 성공 시 타이머 중단
	if (bSuccess)
	{
		Tracker->StopMissionTimer();
	}
	
	// 결과 연출 후 UI 전환을 위한 딜레이 처리
	FTimerHandle TimerHandle;
	float DelayTime = 1.0f;
	GetWorldTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateWeakLambda(this, [this, MissionID, Tracker, Progress]()
		{
			// Tracker와 Progress가 유효한지 먼저 확인
			if (Tracker && Tracker->IsValidLowLevel() && Progress && Progress->IsValidLowLevel())
			{
				// Tracker는 숨기기
				Tracker->SetVisibility(ESlateVisibility::Hidden);

				// 미션 타입에 따른 Progress 위젯 표시 여부 결정
				auto* Subsystem = GetGameInstance()->GetSubsystem<UMSMissionDataSubsystem>();
				if (const FMSMissionRow* MissionData = Subsystem ? Subsystem->Find(MissionID) : nullptr)
				{
					if (MissionData->MissionType != EMissionType::Boss)
					{
						Progress->SetVisibility(ESlateVisibility::Visible);
					}
				}
			}
		}), DelayTime, false
	);
}

// 서버에서 계산된 정규화된 진행 값을 UI에 반영
void AMSPlayerController::OnMissionProgressChanged(float Normalized)
{
	if (!HUDWidgetInstance) return;

	auto* Tracker = HUDWidgetInstance->GetMissionTrackerWidget();
	if (!Tracker) return;
	Tracker->SetTargetHpProgress(Normalized);
}

void AMSPlayerController::ClientShowLoadingWidget_Implementation()
{
	UMSLevelManagerSubsystem* LevelManager = GetGameInstance()->GetSubsystem<UMSLevelManagerSubsystem>();
	if (LevelManager)
	{
		LevelManager->ShowLoadingWidget();
	}
}