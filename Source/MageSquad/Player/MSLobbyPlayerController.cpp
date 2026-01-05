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

void AMSLobbyPlayerController::Server_SelectCharacter_Implementation(
	TSubclassOf<AMSPlayerCharacter> SelectedClass
)
{
	if (!SelectedClass)
		return;

	auto* CharacterDataManager = GetGameInstance()->GetSubsystem<UMSCharacterDataSubsystem>();
	if (!CharacterDataManager || CharacterDataManager->GetAllCharacter().Num() <= 0)
	{
		return;
	}

	FTransform TM = GetPawn()
		? GetPawn()->GetActorTransform()
		: FTransform::Identity;

	if (APawn* PlayerOldPawn = GetPawn())
	{
		PlayerOldPawn->Destroy();
	}

	APawn* NewPawn = GetWorld()->SpawnActor<APawn>(
		SelectedClass,
		TM
	);

	Possess(NewPawn);

	const FUniqueNetIdRepl NetId = PlayerState->GetUniqueId();
	if (!NetId.IsValid())
		return;

	UMSCharacterDataSubsystem* Cache = GetGameInstance()->GetSubsystem<UMSCharacterDataSubsystem>();
	if (!Cache) return;
	Cache->CacheSelectedCharacterForPlayer(NetId, SelectedClass);

}