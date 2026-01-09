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
#include "Widgets/GameProgress/MSGameProgressWidget.h"
#include "System/MSMissionDataSubsystem.h"
#include "Widgets/LevelUp/MSLevelUpPanel.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "MSFunctionLibrary.h"

#include "Net/UnrealNetwork.h"

#include "EngineUtils.h"
#include "DataStructs/MSMissionProgressUIData.h"
#include "SignificanceManager.h"

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

void AMSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!IsLocalController()) return;

	// 관전 입력 액션 바인딩
	UEnhancedInputComponent* Subsystem = Cast<UEnhancedInputComponent>(InputComponent);
	if (!Subsystem) return;

	if (SpectatePrevAction)
	{
		Subsystem->BindAction(SpectatePrevAction, ETriggerEvent::Started, this, &AMSPlayerController::OnSpectatePrevAction);
	}

	if (SpectateNextAction)
	{
		Subsystem->BindAction(SpectateNextAction, ETriggerEvent::Started, this, &AMSPlayerController::OnSpectateNextAction);
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

void AMSPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AMSPlayerController, SpectateTargetActor, COND_OwnerOnly);
}

void AMSPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 서버에서는 모든 플레이어 위치로 업데이트
	if (HasAuthority())
	{
		if (USignificanceManager* SigManager = USignificanceManager::Get(GetWorld()))
		{
			TArray<FTransform> TransformArray;

			// 모든 플레이어의 위치를 수집
			for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
			{
				if (const APlayerController* PC = It->Get())
				{
					if (APawn* OwnerPawn = PC->GetPawn())
					{
						TransformArray.Add(OwnerPawn->GetActorTransform());
					}
				}
			}

			if (TransformArray.Num() > 0)
			{
				SigManager->Update(TArrayView<FTransform>(TransformArray));
			}
		}
		return; // 서버는 여기서 끝
	}

	// 클라이언트는 렌더링 최적화용으로만 사용
	if (IsLocalController())
	{
		if (USignificanceManager* SigManager = USignificanceManager::Get(GetWorld()))
		{
			TArray<FTransform> TransformArray;

			FVector ViewLocation;
			FRotator ViewRotation;
			GetPlayerViewPoint(ViewLocation, ViewRotation);

			TransformArray.Add(FTransform(ViewRotation, ViewLocation));
			SigManager->Update(TArrayView<FTransform>(TransformArray));
		}
	}
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
			if (false == GS->OnMissionChanged.IsBoundToObject(this))
				GS->OnMissionChanged.AddUObject(this, &AMSPlayerController::OnMissionChanged);
			if (false == GS->OnMissionFinished.IsBoundToObject(this))
				GS->OnMissionFinished.AddUObject(this, &AMSPlayerController::OnMissionFinished);
			if (false == GS->OnMissionProgressChanged.IsBoundToObject(this))
				GS->OnMissionProgressChanged.AddUObject(this, &AMSPlayerController::OnMissionProgressChanged);
			if (false == GS->OnBossSpawnCutsceneStateChanged.IsBoundToObject(this))
			{
				//GS->OnBossSpawnCutsceneStateChanged.AddUObject(this, &AMSPlayerController::OnMissionProgressChanged);
			}
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

void AMSPlayerController::SetSpectateTarget_Server(AActor* NewTarget)
{
	if (!HasAuthority()) return;

	// 관전 대상이 유효한지 확인
	if (NewTarget && !UMSFunctionLibrary::IsValidSpectateTargetActor(NewTarget))
	{
		NewTarget = nullptr;
	}

	// 관전 대상 변경
	SpectateTargetActor = NewTarget;

	// 리슨 서버(호스트)는 관전 대상 변경 OnRep 호출이 안되므로, 직접 호출
	if (IsLocalController())
	{
		// 해당 관전 대상이 유효하면 해당 대상으로 관전 카메라 전환
		if (SpectateTargetActor)
		{
			SetSpectateViewTarget(SpectateTargetActor);
		}
		// 관전 대상이 유효하지 않으면 내 Pawn으로 관전 카메라 복귀
		else if (APawn* P = GetPawn())
		{
			SetSpectateViewTarget(P);
		}
	}

	ForceNetUpdate();
}

void AMSPlayerController::SetSpectateViewTarget(AActor* NewTarget)
{
	if (!NewTarget) return;

	// 현재 컷씬 재생중이라면 관전 타깃만 갱신하고 카메라 전환은 보류
	PendingViewTarget = NewTarget;
	if (!bBossCutsceneActive)
	{
		// 관전 전용 전환 연출
		SetViewTargetWithBlend(NewTarget, SpectateBlendTime, SpectateBlendFunction);
	}

	// HUD 위젯에 관전 UI 레이아웃을 설정하도록 호출
	if (HUDWidgetInstance)
	{
		// 플레이어의 관전 상태를 전달
		AMSPlayerCharacter* MSPlayer = Cast<AMSPlayerCharacter>(GetPawn());
		HUDWidgetInstance->SetSpectateUI(MSPlayer ? MSPlayer->GetSpectating() : false, UMSFunctionLibrary::GetTargetPlayerNameText(NewTarget));
	}
}

void AMSPlayerController::SetBossCutsceneActive(bool bActive)
{
	// 보스 컷씬 진행 여부 초기화
	bBossCutsceneActive = bActive;

	// 보스 컷씬 진행 여부에 따라 보류된 카메라 타깃 적용
	if (!bBossCutsceneActive)
	{
		ApplyPendingViewTarget();
	}
}

AActor* AMSPlayerController::GetDesiredViewTarget() const
{
	// 보류된 카메라가 있으면 보류된 카메라를 반환
	if (PendingViewTarget.IsValid())
	{
		return PendingViewTarget.Get();
	}

	// 현재 관전 대상이 유효하면 현재 관전 대상 반환
	if (SpectateTargetActor && IsValid(SpectateTargetActor))
	{
		return SpectateTargetActor.Get();
	}

	// 전환할 카메라가 없으면 본인 폰 반환
	return GetPawn();
}

void AMSPlayerController::ApplyPendingViewTarget()
{
	// 보스 컷씬 진행중이면 종료
	if (bBossCutsceneActive) return;

	// 복귀할 카메라가 있다면 카메라 가져오기
	AActor* Target = GetDesiredViewTarget();
	if (!Target) return;

	// 복귀할 카메라로 뷰 타깃 전환
	SetViewTargetWithBlend(Target, SpectateBlendTime, SpectateBlendFunction);

	// HUD 위젯에 관전 UI 레이아웃을 설정하도록 호출
	if (HUDWidgetInstance)
	{
		// 플레이어의 관전 상태를 전달
		AMSPlayerCharacter* MSPlayer = Cast<AMSPlayerCharacter>(GetPawn());
		HUDWidgetInstance->SetSpectateUI(MSPlayer ? MSPlayer->GetSpectating() : false, UMSFunctionLibrary::GetTargetPlayerNameText(Target));
	}
}

void AMSPlayerController::ApplyLocalInputState(bool bDead)
{
	if (!IsLocalController()) return;

	ULocalPlayer* LP = GetLocalPlayer();
	if (!LP) return;

	// 향상된 입력 시스템 가져오기
	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!Subsystem) return;

	// 사망했으면 관전 IMC로 교체
	if (bDead)
	{
		// 기본 게임 플레이 IMC 제거
		if (DefaultIMC)
		{
			Subsystem->RemoveMappingContext(DefaultIMC);
		}

		// 관전 전용 IMC 추가
		if (SpectateIMC)
		{
			Subsystem->AddMappingContext(SpectateIMC, 10);
		}
	}
	// 부활했으면 관전 IMC 제거 및 기존 IMC 할당
	else
	{
		// 관전 전용 IMC 제거
		if (SpectateIMC)
		{
			Subsystem->RemoveMappingContext(SpectateIMC);
		}

		// 기본 게임 플레이 IMC 추가
		if (DefaultIMC)
		{
			Subsystem->AddMappingContext(DefaultIMC, 0);
		}
	}
}

void AMSPlayerController::EnsureValidSpectateTarget_Server()
{
	if (!HasAuthority()) return;

	// 관전 대상이 유효하면 종료
	if (SpectateTargetActor && UMSFunctionLibrary::IsValidSpectateTargetActor(SpectateTargetActor)) return;

	// 현재 내 캐릭터가 관전 상태가 아니면 종료
	AMSPlayerCharacter* MyChar = Cast<AMSPlayerCharacter>(GetPawn());
	if (!MyChar || !MyChar->GetSpectating()) return;

	// 월드의 모든 플레이어를 순회하며 생존중인 캐릭터를 찾아 저장
	AActor* Fallback = nullptr;
	for (APlayerController* PC : TActorRange<APlayerController>(GetWorld()))
	{
		AMSPlayerCharacter* Char = PC ? Cast<AMSPlayerCharacter>(PC->GetPawn()) : nullptr;
		if (Char && Char != MyChar && !Char->GetIsDead())
		{
			Fallback = Char;
			break;
		}
	}

	// 월드의 생존중인 캐릭터로 관전 대상을 전환
	SetSpectateTarget_Server(Fallback);
}

void AMSPlayerController::RequestChangeSpectate(int32 Direction)
{
	if (HasAuthority())
	{
		// 서버는 바로 처리
		HandleChangeSpectate_Server(Direction);
	}
	else
	{
		// 클라이언트는 서버에게 관전 대상 변경 요청
		ServerRPCChangeSpectate(Direction);
	}
}

void AMSPlayerController::HandleChangeSpectate_Server(int32 Direction)
{
	if (!HasAuthority()) return;

	// 현재 내 캐릭터가 관전 상태가 아니면 종료
	AMSPlayerCharacter* MyChar = Cast<AMSPlayerCharacter>(GetPawn());
	if (!MyChar || !MyChar->GetSpectating()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	// 월드의 모든 플레이어를 순회하며 생존중인 캐릭터를 모두 찾아 저장
	TArray<AMSPlayerCharacter*> AliveCharacters;
	for (APlayerController* PC : TActorRange<APlayerController>(World))
	{
		AMSPlayerCharacter* Char = PC ? Cast<AMSPlayerCharacter>(PC->GetPawn()) : nullptr;
		if (Char && Char != MyChar && !Char->GetIsDead())
		{
			AliveCharacters.Add(Char);
		}
	}

	// 생존중인 대상이 없으면 종료
	if (AliveCharacters.Num() == 0)
	{
		SetSpectateTarget_Server(nullptr);
		return;
	}

	// 현재 관전 대상 가져오기
	AActor* CurrentTarget = SpectateTargetActor ? SpectateTargetActor.Get() : GetViewTarget();

	// 생존중인 캐릭터 목록에 현재 관전 대상이 있는지 검사 + 인덱스 가져오기
	int32 CurrentIndex = AliveCharacters.IndexOfByKey(CurrentTarget);

	// 관전 대상이 유효하면 해당 관전 대상 인덱스를 기준으로 모듈러 연산
	const int32 NewIndex = (CurrentIndex == INDEX_NONE) ? 0
		: (CurrentIndex + Direction + AliveCharacters.Num()) % AliveCharacters.Num();

	// 다음 관전 대상으로 관전 대상 설정
	SetSpectateTarget_Server(AliveCharacters[NewIndex]);
}

void AMSPlayerController::ServerRPCChangeSpectate_Implementation(int32 Direction)
{
	if (!HasAuthority()) return;

	HandleChangeSpectate_Server(Direction);
}

void AMSPlayerController::CycleSpectateTarget(int32 Direction)
{
	if (!IsLocalController() && !HasAuthority()) return;

	// 관전 대상 변경 요청
	RequestChangeSpectate(Direction);
}

void AMSPlayerController::OnRep_SpectateTargetActor()
{
	if (!IsLocalController()) return;

	// 관전 대상이 변경된 경우, 해당 관전 대상이 유효하면 해당 대상으로 관전 카메라 전환
	if (SpectateTargetActor && IsValid(SpectateTargetActor))
	{
		SetSpectateViewTarget(SpectateTargetActor);
	}
	// 관전 대상이 유효하지 않으면 내 Pawn으로 관전 카메라 복귀
	else if (APawn* P = GetPawn())
	{
		SetSpectateViewTarget(P);
	}
}

void AMSPlayerController::OnSpectatePrevAction(const FInputActionValue& Value)
{
	// 로컬 컨트롤러가 아닌 경우 종료
	if (!IsLocalController()) return;

	// 관전 대상 변경 요청 (이전 대상으로)
	RequestChangeSpectate(-1);
}

void AMSPlayerController::OnSpectateNextAction(const FInputActionValue& Value)
{
	// 로컬 컨트롤러가 아닌 경우 종료
	if (!IsLocalController()) return;

	// 관전 대상 변경 요청 (다음 대상으로)
	RequestChangeSpectate(+1);
}

void AMSPlayerController::Client_CloseSkillLevelUpChoices_Implementation(int32 SessionId, bool bKeepPaused)
{
	if (!IsLocalController()) return;

	if (LevelUpPanelInstance)
	{
		LevelUpPanelInstance->RemoveFromParent();
		LevelUpPanelInstance = nullptr;
	}

	if (!bKeepPaused)
	{
		SetPause(false);
	}

	if (!bKeepPaused)
	{
		FInputModeGameAndUI InputMode;
		SetInputMode(InputMode);
	}
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
void AMSPlayerController::OnMissionFinished(int32 MissionID, bool bSuccess)
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
void AMSPlayerController::OnMissionProgressChanged(const FMSMissionProgressUIData& Data)
{
	if (!HUDWidgetInstance) return;

	auto* Tracker = HUDWidgetInstance->GetMissionTrackerWidget();
	if (!Tracker) return;

	Tracker->UpdateProgress(Data);
}

void AMSPlayerController::ClientShowLoadingWidget_Implementation()
{
	UMSLevelManagerSubsystem* LevelManager = GetGameInstance()->GetSubsystem<UMSLevelManagerSubsystem>();
	if (LevelManager)
	{
		LevelManager->ShowLoadingWidget();
	}
}

void AMSPlayerController::ClientRPCShowEndGameWidget_Implementation(TSubclassOf<UUserWidget> WidgetClass, int32 ZOrder)
{
	if (!IsLocalController() || !WidgetClass) return;

	// 기존 위젯 있으면 교체(승리/패배 전환 등)
	if (EndGameWidget)
	{
		EndGameWidget->RemoveFromParent();
		EndGameWidget = nullptr;
	}

	// 위젯 생성 및 표시
	EndGameWidget = CreateWidget<UUserWidget>(this, WidgetClass);
	if (EndGameWidget)
	{
		EndGameWidget->AddToViewport(ZOrder);
	}
}
