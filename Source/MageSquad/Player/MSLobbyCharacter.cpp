// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MSLobbyCharacter.h"
#include "Components/WidgetComponent.h"
#include "Widgets/Lobby/MSLobbyPlayerEntryWidget.h"
#include "System/MSSteamManagerSubsystem.h"
#include "GameFramework/PlayerController.h" 
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "MageSquad.h"
// Sets default values
AMSLobbyCharacter::AMSLobbyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
	bIsHostIconVisible = false;
	bIsReady = false;

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
		LobbyPlayerEntryWidget = Cast<UMSLobbyPlayerEntryWidget>(LobbyPlayerEntryWidgetComponent->GetWidget());
		if (LobbyPlayerEntryWidget)
		{
			if (HasAuthority())
			{
				//닉네임 지정하기
				APlayerState* PS = GetPlayerState<APlayerState>();
				if (PS)
				{
					UserNickName = PS->GetPlayerName();
				}
				if (UMSSteamManagerSubsystem* SteamManager = GetGameInstance()->GetSubsystem<UMSSteamManagerSubsystem>())
				{
					//스팀에 연결되지 않으면 Player로 지정
					if (!SteamManager->IsSteamConnected())
					{
						UserNickName = TEXT("Player");
					}
				}

				LobbyPlayerEntryWidget->SetTextNickName(UserNickName);
				if (IsLocallyControlled())
				{
					bIsHostIconVisible = true;
					LobbyPlayerEntryWidget->ShowHostIcon();
				}
			}
			else //클라이언트 실행
			{
				//bIsHostIconVisible가 동기화 된후 BeginPlay가 나중에 실행되는 경우가 있어서 
				OnRep_IsHostIconVisible();
			}
		}
	}
}

void AMSLobbyCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

}

void AMSLobbyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMSLobbyCharacter, UserNickName);
	DOREPLIFETIME(AMSLobbyCharacter, bIsHostIconVisible);
}

void AMSLobbyCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	
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

void AMSLobbyCharacter::OnRep_UserNickName()
{
	if (LobbyPlayerEntryWidget)
	{
		LobbyPlayerEntryWidget->SetTextNickName(UserNickName);
	}
}

void AMSLobbyCharacter::OnRep_IsHostIconVisible()
{
	MS_LOG(LogMSNetwork, Log, TEXT("%s"), TEXT("AMSLobbyCharacter OnRep_IsHostIconVisible"));
	if (LobbyPlayerEntryWidget && bIsHostIconVisible)
	{
		UE_LOG(LogTemp, Log, TEXT("true"));
		LobbyPlayerEntryWidget->ShowHostIcon();
	}
}
