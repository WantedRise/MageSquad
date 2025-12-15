// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MSLobbyPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "MageSquad.h"
#include "MSLobbyPlayerState.h"
#include <System/MSSteamManagerSubsystem.h>
#include "Widgets/Lobby/MSLobbyMainWidget.h"
#include "Widgets/Lobby/MSLobbyReadyWidget.h"
#include <GameStates/MSLobbyGameState.h>

AMSLobbyPlayerController::AMSLobbyPlayerController()
{
	//카메라 시점 제어를 위한 설정
	bAutoManageActiveCameraTarget = false;
}

void AMSLobbyPlayerController::InitPlayerState()
{
	Super::InitPlayerState();

	AMSLobbyPlayerState* PS = GetPlayerState<AMSLobbyPlayerState>();

	if (nullptr == PS)
	{
		MS_LOG(LogMSNetwork, Error, TEXT("%s"), TEXT("nullptr == AMSLobbyPlayerState"));
		return;
	}

	UMSSteamManagerSubsystem* SteamManager = GetGameInstance()->GetSubsystem<UMSSteamManagerSubsystem>();

	if (nullptr == SteamManager)
	{
		MS_LOG(LogMSNetwork, Error, TEXT("%s"), TEXT("nullptr == UMSSteamManagerSubsystem"));
		return;
	}

	//스팀에 연결되지 않으면 Player로 지정
	if (!SteamManager->IsSteamConnected())
	{
		PS->SetUserNickName(TEXT("Player"));
	}
	else
	{
		PS->SetUserNickName(PS->GetPlayerName());
	}

	if (IsLocalController())
	{
		// 이 플레이어가 호스트(방장)
		PS->SetHost(true);
	}
}

void AMSLobbyPlayerController::ServerRequestSetReady_Implementation(bool bNewReady)
{
	if (AMSLobbyPlayerState* PS = GetPlayerState<AMSLobbyPlayerState>())
	{
		PS->SetReady(bNewReady);
	}
}

void AMSLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		FName TargetTag = TEXT("LobbyCamera");

		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), TargetTag, FoundActors);

		if (FoundActors.Num() > 0)
		{
			//LobbyCamera 액터로 시점 전환
			SetViewTargetWithBlend(FoundActors[0]);
		}

	
		CreateLobbyUI();
	}
	
}

void AMSLobbyPlayerController::CreateLobbyUI()
{
	if (LobbyMainWidget || !LobbyMainWidgetClass)
		return;

	LobbyMainWidget = CreateWidget<UMSLobbyMainWidget>(
		this,
		LobbyMainWidgetClass
	);

	if (LobbyMainWidget)
	{
		LobbyMainWidget->AddToViewport();

		// 입력 모드 UI
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(LobbyMainWidget->TakeWidget());
		SetInputMode(InputMode);
		bShowMouseCursor = true;

		/* 로컬 플레이어 전용 로비 Ready UI 바인딩
			- 시간 카운트다운은 GameState에서 관리 (전역 상태)
			- 준비 여부는 PlayerState에서 관리 (개별 상태)
			- UI는 변경 이벤트만 수신한다.
		*/
		// 준비 시간(초) 변경 시 UI 갱신
		// GameState의 RemainingReadySeconds 변경 → UI 카운트다운 반영
		if (AMSLobbyGameState* GS = GetWorld()->GetGameState<AMSLobbyGameState>())
		{
			GS->OnReadyTimeChanged.AddUObject(
				LobbyMainWidget->WBP_MSLobbyReady,
				&UMSLobbyReadyWidget::OnReadyTimeChanged
			);
		}
		// 준비 상태 변경 시 UI 전환
		// 로비에 최소 1명 이상이 준비 상태인지에 따라 Ready UI를 전환한다.
		if (AMSLobbyPlayerState* PS = GetPlayerState<AMSLobbyPlayerState>())
		{
			PS->OnLobbyReadyStateChanged.AddUObject(LobbyMainWidget->WBP_MSLobbyReady, &UMSLobbyReadyWidget::ApplyReadyStateUI);
		}
	}

}