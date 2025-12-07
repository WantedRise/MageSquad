// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/MSSteamManagerSubsystem.h"
#include "OnlineSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Interfaces/OnlineExternalUIInterface.h"

const FName SESSION_NAME_GAME = FName(TEXT("MSSteamSeesion"));

void UMSSteamManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{

}

void UMSSteamManagerSubsystem::Deinitialize()
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->DestroySession(SESSION_NAME_GAME);
		SessionInterface->OnCreateSessionCompleteDelegates.RemoveAll(this);
		SessionInterface->OnDestroySessionCompleteDelegates.RemoveAll(this);
		SessionInterface->OnFindSessionsCompleteDelegates.RemoveAll(this);
		SessionInterface->OnJoinSessionCompleteDelegates.RemoveAll(this);
	}
}

void UMSSteamManagerSubsystem::CreateSteamSession(bool bIsLAN, int32 MaxPlayers)
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		SessionInterface = OnlineSub->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UMSSteamManagerSubsystem::OnCreateSessionComplete);
			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UMSSteamManagerSubsystem::OnDestroySessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UMSSteamManagerSubsystem::OnFindSessionsComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UMSSteamManagerSubsystem::OnJoinSessionComplete);

			FOnlineSessionSettings SessionSettings;
			SessionSettings.bIsLANMatch = bIsLAN;
			SessionSettings.bUsesPresence = true;
			SessionSettings.NumPublicConnections = MaxPlayers;
			SessionSettings.bAllowJoinViaPresence = true;
			SessionSettings.bIsDedicated = false;        // 전용 서버가 아님
			SessionSettings.bAllowJoinInProgress = true; // 게임 중 참여 허용
			SessionSettings.bShouldAdvertise = true;     // 친구들이 세션을 찾을 수 있도록 알림
			SessionSettings.bUseLobbiesIfAvailable = true; // 로비 사용
			SessionInterface->CreateSession(0, SESSION_NAME_GAME, SessionSettings);
		}
	}
}

void UMSSteamManagerSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("Session '%s' created successfully!"), *SessionName.ToString());
		FindSteamSessions(true);
	}
}
void UMSSteamManagerSubsystem::DestroySteamSession()
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->DestroySession(SESSION_NAME_GAME);
	}
}
void UMSSteamManagerSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->OnCreateSessionCompleteDelegates.RemoveAll(this);
		SessionInterface->OnDestroySessionCompleteDelegates.RemoveAll(this);
		SessionInterface->OnFindSessionsCompleteDelegates.RemoveAll(this);
		SessionInterface->OnJoinSessionCompleteDelegates.RemoveAll(this);
	}
	UE_LOG(LogTemp, Warning, TEXT("Session '%s' OnDestroySessionComplete! %s"), *SessionName.ToString(),bWasSuccessful?TEXT("true"):TEXT("false"));
}

void UMSSteamManagerSubsystem::FindSteamSessions(bool bIsLAN)
{
	UE_LOG(LogTemp, Warning, TEXT("FindSteamSessions"));
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnlineSub"));
		if (IOnlineExternalUIPtr UIPtr = OnlineSub->GetExternalUIInterface())
		{
			if (!SessionInterface.IsValid())
			{
				UE_LOG(LogTemp, Warning, TEXT("SessionInterface"));
				return;
			}
			if (nullptr == SessionInterface->GetNamedSession(SESSION_NAME_GAME))
			{
				UE_LOG(LogTemp, Warning, TEXT("GetNamedSession"));
				return;
			}

			UE_LOG(LogTemp, Warning, TEXT("UIPtr %s %s"), UIPtr->ShowInviteUI(0, SESSION_NAME_GAME) ? TEXT("true") : TEXT("false"), SessionInterface->GetNamedSession(SESSION_NAME_GAME)->SessionInfo.IsValid()? TEXT("true") : TEXT("false"));
		}
	}
}

void UMSSteamManagerSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{

}
void UMSSteamManagerSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{

}
void UMSSteamManagerSubsystem::OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult)
{

}
