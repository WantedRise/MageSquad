// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MSLobbyCharacter.h"
#include "Components/WidgetComponent.h"
#include "Widgets/Lobby/MSLobbyPlayerEntryWidget.h"
#include "System/MSSteamManagerSubsystem.h"
#include "GameFramework/PlayerController.h" 
#include "GameFramework/PlayerState.h"
#include "MageSquad.h"
#include "MSLobbyPlayerState.h"
#include "DataStructs/MSCharacterData.h"
#include <System/MSCharacterDataSubsystem.h>
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AMSLobbyCharacter::AMSLobbyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;

	LobbyPlayerEntryWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("LobbyPlayerEntryWidgetComponent"));
	LobbyPlayerEntryWidgetComponent->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UMaterial> MaterialFinder(TEXT("/Game/Level/Materials/Lobby/Widget3DPassThrough.Widget3DPassThrough"));
	if (MaterialFinder.Succeeded())
	{
		Widget3DPassThroughMaterial = MaterialFinder.Object;
	}

	StaffMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Staff"));
	StaffMesh->SetupAttachment(GetMesh(), StaffAttachSocketName);
	StaffMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StaffMesh->SetGenerateOverlapEvents(false);
	StaffMesh->PrimaryComponentTick.bCanEverTick = false;
	StaffMesh->PrimaryComponentTick.bStartWithTickEnabled = false;
	StaffMesh->bReceivesDecals = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
}

// Called when the game starts or when spawned
void AMSLobbyCharacter::BeginPlay()
{
	Super::BeginPlay();
	MS_LOG(LogMSNetwork, Log, TEXT("%s"), TEXT("AMSLobbyCharacter BeginPlay"));

	if (LobbyPlayerEntryWidgetComponent)
	{
		LobbyPlayerEntryWidgetComponent->SetMaterial(0, Widget3DPassThroughMaterial);
	
		LobbyPlayerEntryWidgetComponent->InitWidget();

		LobbyPlayerEntryWidget = Cast<UMSLobbyPlayerEntryWidget>(LobbyPlayerEntryWidgetComponent->GetWidget());

		if (AMSLobbyPlayerState* PS = GetPlayerState<AMSLobbyPlayerState>())
		{
			UpdateHostUI(PS->IsHost());
			UpdateUserNickNameUI(PS->GetUserNickName());
		}
	}
}

void AMSLobbyCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	MS_LOG(LogMSNetwork, Log, TEXT("%s"), TEXT("AMSLobbyCharacter::PossessedBy"));

	if (AMSLobbyPlayerState* PS = GetPlayerState<AMSLobbyPlayerState>())
	{
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
	}

	InitializeLobbyCharacterFromPlayerState();
	MS_LOG(LogMSNetwork, Log, TEXT("PossessedBy %s"), *GetActorRotation().ToString());
}

void AMSLobbyCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	MS_LOG(LogMSNetwork, Log, TEXT("%s"), TEXT("AMSLobbyCharacter OnRep_PlayerState"));

	InitializeLobbyCharacterFromPlayerState();
}

void AMSLobbyCharacter::InitializeLobbyCharacterFromPlayerState()
{
	if (AMSLobbyPlayerState* PS = GetPlayerState<AMSLobbyPlayerState>())
	{
		UpdateUserNickNameUI(PS->GetUserNickName());
		UpdateHostUI(PS->IsHost());
		UpdateReadyStatusUI(PS->IsReady());
		// 준비 상태 변경 시 UI 전환
		// PlayerState의 Ready 상태 변경 → Ready / Cancel UI 토글
		PS->OnLobbyReadyStateChanged.AddUObject(this, &AMSLobbyCharacter::UpdateReadyStatusUI);
	}
}

// Called every frame
void AMSLobbyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMSLobbyCharacter::UpdateUserNickNameUI(const FString& InUserNickName)
{
	MS_LOG(LogMSNetwork, Log, TEXT("%s"), TEXT("AMSLobbyCharacter UpdateUserNickNameUI"));
	if (LobbyPlayerEntryWidget)
	{
		LobbyPlayerEntryWidget->SetTextNickName(InUserNickName);
	}
}

void AMSLobbyCharacter::UpdateHostUI(bool bReady)
{
	MS_LOG(LogMSNetwork, Log, TEXT("%s"), TEXT("AMSLobbyCharacter UpdateHostUI"));
	UE_LOG(LogMSNetwork, Log, TEXT("%s"), bReady ? TEXT("true"): TEXT("false"));
	if (LobbyPlayerEntryWidget && bReady)
	{
		LobbyPlayerEntryWidget->ShowHostIcon();
	}
}

void AMSLobbyCharacter::UpdateReadyStatusUI(bool bReady)
{
	MS_LOG(LogMSNetwork, Log, TEXT("%s"), TEXT("Check"));

	if (LobbyPlayerEntryWidget)
	{
		UE_LOG(LogMSNetwork, Log, TEXT("%s"), TEXT("true"));
		LobbyPlayerEntryWidget->SetTextReadyStatus(bReady);
	}
}