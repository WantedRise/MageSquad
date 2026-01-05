// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MSLobbyPlayerState.h"
#include "MageSquad.h"
#include "Player/MSLobbyCharacter.h"
#include "Player/MSLobbyPlayerController.h"
#include "Net/UnrealNetwork.h"
#include <System/MSSteamManagerSubsystem.h>
#include <GameModes/MSLobbyGameMode.h>
#include <System/MSLevelManagerSubsystem.h>

AMSLobbyPlayerState::AMSLobbyPlayerState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bAlwaysRelevant = true;
	bIsHost = false;
	bIsReady = false;
}

void AMSLobbyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMSLobbyPlayerState, UserNickName);
	DOREPLIFETIME(AMSLobbyPlayerState, bIsHost);
	DOREPLIFETIME(AMSLobbyPlayerState, bIsReady);
}

void AMSLobbyPlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();


}
// 준비 상태 설정 변경 시 GameMode에 알려 전체 준비 상태를 재계산한다.
void AMSLobbyPlayerState::SetReady(bool bNewReady)
{
	if (!HasAuthority())
	{
		return;
	}
	MS_LOG(LogMSNetwork, Log, TEXT("%s %s"), *FString::FromInt(bIsReady), bNewReady ? TEXT("true") : TEXT("false"));
	bIsReady = bNewReady;
	OnRep_IsReady();

	if (AMSLobbyGameMode* GM = GetWorld()->GetAuthGameMode<AMSLobbyGameMode>())
	{
		GM->HandlePlayerReadyStateChanged();
	}
}

void AMSLobbyPlayerState::SetHost(bool bNewHost)
{
	if (!HasAuthority())
	{
		return;
	}

	if (bIsHost != bNewHost)
	{
		bIsHost = bNewHost;
		OnRep_IsHost();
	}
}

void AMSLobbyPlayerState::SetUserNickName(const FString& NewNickName)
{
	if (!HasAuthority())
	{
		return;
	}

	if (UserNickName.Compare(NewNickName)!=0)
	{
		UserNickName = NewNickName;
	}
}

AMSLobbyCharacter* AMSLobbyPlayerState::GetLobbyCharacter() const
{
	return Cast<AMSLobbyCharacter>(GetPawn());
}

void AMSLobbyPlayerState::OnRep_UserNickName()
{
	MS_LOG(LogMSNetwork, Log, TEXT("%s"), TEXT("AMSLobbyPlayerState OnRep_UserNickName"));

}

void AMSLobbyPlayerState::OnRep_IsHost()
{
	MS_LOG(LogMSNetwork, Log, TEXT("%s"), TEXT("AMSLobbyPlayerState OnRep_IsHost"));

}

void AMSLobbyPlayerState::OnRep_IsReady()
{
	MS_LOG(LogMSNetwork, Log, TEXT("%s"), TEXT("IsBound "));

	if (OnLobbyReadyStateChanged.IsBound())
	{
		MS_LOG(LogMSNetwork, Log, TEXT("%s"), TEXT("OnLobbyReadyStateChanged "));
		OnLobbyReadyStateChanged.Broadcast(bIsReady);
	}
}

void AMSLobbyPlayerState::SetSelectedCharacter(FName CharacterID)
{
	SelectedCharacterID = CharacterID;
	OnRep_SelectedCharacterID();
}

void AMSLobbyPlayerState::OnRep_SelectedCharacterID()
{
	if (OnCharacterChanged.IsBound())
	{
		OnCharacterChanged.Broadcast(SelectedCharacterID);
	}
}

