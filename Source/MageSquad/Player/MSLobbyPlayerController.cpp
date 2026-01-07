// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MSLobbyPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "MageSquad.h"
#include "MSLobbyPlayerState.h"
#include "System/MSSteamManagerSubsystem.h"
#include "Widgets/Lobby/MSLobbyMainWidget.h"
#include "Widgets/Lobby/MSLobbyReadyWidget.h"
#include "GameStates/MSLobbyGameState.h"
#include <System/MSLevelManagerSubsystem.h>
#include "Widgets/Lobby/MSCharacterSelectWidget.h"
#include "DataStructs/MSCharacterData.h"
#include <Interfaces/CharacterAppearanceInterface.h>
#include <System/MSCharacterDataSubsystem.h>
#include "Player/MSPlayerCharacter.h"
#include "DataAssets/Player/DA_CharacterData.h"
#include "Player/MSLobbyCharacter.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "GameModes/MSLobbyGameMode.h"

AMSLobbyPlayerController::AMSLobbyPlayerController()
{
	//카메라 시점 제어를 위한 설정
	bAutoManageActiveCameraTarget = false;
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

		LobbyCameraActor = GetViewTarget();
	}
}

void AMSLobbyPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	if (LobbyMainWidget)
	{
		if (UMSCharacterSelectWidget* CharacterSelectWidget = LobbyMainWidget->GetCharacterSelectWidget())
		{
			//CharacterSelectWidget->UpdatePlayerState();
		}
	}
}

void AMSLobbyPlayerController::BeginPlayingState()
{
	Super::BeginPlayingState();
	if (!IsLocalController()) return;
	if (!GetPawn()) return;
	AMSLobbyCharacter* LobbyCharacter = Cast<AMSLobbyCharacter>(GetPawn());
	if (!LobbyCharacter) return;
	check(LobbyCharacter->GetPlayerCameraComponent());
	FTransform CameraWorldTransform = LobbyCharacter->GetPlayerCameraComponent()->GetComponentTransform();
	// 월드 카메라 생성
	FActorSpawnParameters SpawnParams;
	CharacterCameraActor = GetWorld()->SpawnActor<ACameraActor>(CameraWorldTransform.GetLocation(), CameraWorldTransform.Rotator(), SpawnParams);
	UCameraComponent* CameraComp = CharacterCameraActor->GetCameraComponent();
	check(CameraComp);
	CameraComp->bConstrainAspectRatio = false;
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
		//FInputModeUIOnly InputMode;
		//InputMode.SetWidgetToFocus(LobbyMainWidget->TakeWidget());
		//SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
}

void AMSLobbyPlayerController::Server_SelectCharacter_Implementation(FName InCharacterId)
{
	if (InCharacterId == NAME_None)
		return;

	auto* CharacterDataManager = GetGameInstance()->GetSubsystem<UMSCharacterDataSubsystem>();
	if (!CharacterDataManager || CharacterDataManager->GetAllCharacter().Num() <= 0)
	{
		return;
	}

	FTransform TM = GetPawn() ? GetPawn()->GetActorTransform() : FTransform::Identity;

	if (APawn* PlayerOldPawn = GetPawn())
	{
		PlayerOldPawn->Destroy();
	}

	if (const FMSCharacterSelection* CharacterSelection = CharacterDataManager->FindSelectionByCharacterId(InCharacterId))
	{
		if (CharacterSelection->LobbyCharacterClass)
		{
			APawn* NewPawn = GetWorld()->SpawnActor<APawn>(CharacterSelection->LobbyCharacterClass, TM);
			Possess(NewPawn);
		}
	}

	const FUniqueNetIdRepl NetId = PlayerState->GetUniqueId();
	if (!NetId.IsValid())
		return;

	CharacterDataManager->CacheSelectedCharacter(NetId, InCharacterId);
}

void AMSLobbyPlayerController::SwitchToCharacterCamera()
{
	if (!GetPawn() || !GetViewTarget() || GetViewTarget() == GetPawn()) return;
	
	if (HasAuthority() && IsLocalController())
	{
		AMSLobbyGameMode* LobbyGM = GetWorld()->GetAuthGameMode<AMSLobbyGameMode>();
		check(LobbyGM);
		LobbyGM->SetHiddenPlayerSlots();
	}

	FViewTargetTransitionParams Params;
	Params.BlendTime = 0.4f;
	Params.BlendFunction = EViewTargetBlendFunction::VTBlend_EaseInOut;
	Params.BlendExp = 2.f;
	SetViewTarget(CharacterCameraActor, Params);
}

void AMSLobbyPlayerController::SwitchToLobbyCamera()
{
	if (!LobbyCameraActor) return;

	if (HasAuthority() && IsLocalController())
	{
		AMSLobbyGameMode* LobbyGM = GetWorld()->GetAuthGameMode<AMSLobbyGameMode>();
		check(LobbyGM);
		LobbyGM->SetShowPlayerSlots();
	}

	FViewTargetTransitionParams Params;
	Params.BlendTime = 0.4f;
	Params.BlendFunction = EViewTargetBlendFunction::VTBlend_EaseInOut;
	Params.BlendExp = 2.f;

	SetViewTarget(LobbyCameraActor, Params);
}

void AMSLobbyPlayerController::RequestExitLobby()
{
	if (HasAuthority())
	{
		// Host (Listen Server)
		if (GetWorld())
		{
			GetWorld()->GetAuthGameMode()->ReturnToMainMenuHost();
		}
	}
	else
	{
		// Client
		Server_RequestExitLobby();
	}
}

void AMSLobbyPlayerController::Server_RequestExitLobby_Implementation()
{
	// Client 세션 Leave 후 메인메뉴 이동
	if (UMSSteamManagerSubsystem* SteamManagerSubsystem = GetGameInstance()->GetSubsystem<UMSSteamManagerSubsystem>())
	{
		SteamManagerSubsystem->LeaveSession();
	}

	if (UMSLevelManagerSubsystem* LevelManager = GetGameInstance()->GetSubsystem<UMSLevelManagerSubsystem>())
	{
		ClientTravel(LevelManager->GetMainmenuLevelURL(), TRAVEL_Absolute);
	}
}