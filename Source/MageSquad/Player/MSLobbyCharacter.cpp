// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MSLobbyCharacter.h"
#include "Components/WidgetComponent.h"
#include "Widgets/Lobby/MSLobbyPlayerEntryWidget.h"
#include "System/MSSteamManagerSubsystem.h"
#include "GameFramework/PlayerController.h" 
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "MageSquad.h"
#include "MSLobbyPlayerState.h"
// Sets default values
AMSLobbyCharacter::AMSLobbyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;

	LobbyPlayerEntryWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("LobbyPlayerEntryWidgetComponent"));
	LobbyPlayerEntryWidgetComponent->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UMaterial> MaterialFinder(TEXT("/Game/Level/Materials/Loby/Widget3DPassThrough.Widget3DPassThrough"));
	if (MaterialFinder.Succeeded())
	{
		LobbyPlayerEntryWidgetComponent->SetMaterial(0, MaterialFinder.Object);
	}
}

// Called when the game starts or when spawned
void AMSLobbyCharacter::BeginPlay()
{
	Super::BeginPlay();
	MS_LOG(LogMSNetwork, Log, TEXT("%s"), TEXT("AMSLobbyCharacter BeginPlay"));

	if (LobbyPlayerEntryWidgetComponent)
	{
		LobbyPlayerEntryWidgetComponent->InitWidget();

		LobbyPlayerEntryWidget = Cast<UMSLobbyPlayerEntryWidget>(LobbyPlayerEntryWidgetComponent->GetWidget());
	}
}

void AMSLobbyCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

}

void AMSLobbyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

}

void AMSLobbyCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitializeLobbyCharacterFromPlayerState();
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

// Called to bind functionality to input
void AMSLobbyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

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
	MS_LOG(LogMSNetwork, Log, TEXT("%s"), TEXT("TEST"));

	if (LobbyPlayerEntryWidget)
	{
		UE_LOG(LogMSNetwork, Log, TEXT("%s"), TEXT("true"));
		LobbyPlayerEntryWidget->SetTextReadyStatus(bReady);
	}
}



