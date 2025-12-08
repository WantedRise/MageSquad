// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/MSSteamManagerSubsystem.h"
#include "OnlineSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "GameFramework/PlayerController.h"

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
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UMSSteamManagerSubsystem::OnJoinSessionComplete);
			SessionInterface->OnSessionUserInviteAcceptedDelegates.AddUObject(this, &UMSSteamManagerSubsystem::OnSessionUserInviteAccepted);

			FOnlineSessionSettings SessionSettings;
			SessionSettings.bIsLANMatch = false;
			SessionSettings.bUsesPresence = true;
			SessionSettings.NumPublicConnections = MaxPlayers;
			SessionSettings.bAllowJoinViaPresence = true;
			SessionSettings.bIsDedicated = false;        // 전용 서버가 아님
			SessionSettings.bAllowJoinInProgress = true; // 게임 중 참여 허용
			SessionSettings.bShouldAdvertise = true;     // 친구들이 세션을 찾을 수 있도록 알림
			SessionSettings.bUseLobbiesIfAvailable = true; // 로비 사용
			SessionSettings.Set(FName("Session Type"), FString("AdvertisedSessionHost"), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing); //친구 초대를 위한 설정
			SessionInterface->CreateSession(0, SESSION_NAME_GAME, SessionSettings);
		}
	}
}

void UMSSteamManagerSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("Session '%s' created successfully!"), *SessionName.ToString());
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

void UMSSteamManagerSubsystem::ShowFriendInvitationScreen()
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
				UE_LOG(LogTemp, Warning, TEXT("NULL SessionInterface"));
				return;
			}
			if (nullptr == SessionInterface->GetNamedSession(SESSION_NAME_GAME))
			{
				UE_LOG(LogTemp, Warning, TEXT("NULL GetNamedSession"));
				return;
			}

			UIPtr->ShowInviteUI(0, SESSION_NAME_GAME);
		}
	}
}

void UMSSteamManagerSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	FString TravelURL;
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PlayerController && SessionInterface->GetResolvedConnectString(SessionName, TravelURL))
	{
		FString MapPath = TEXT("/Game/Level/LobyLevel");
		TravelURL += MapPath;
		UE_LOG(LogTemp, Warning, TEXT("ClientTravel. Joining session Complete... %s"),*TravelURL);
		PlayerController->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Joining session Fail..."));
	}
}

void UMSSteamManagerSubsystem::OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invite accepted. Joining session..."));
		SessionInterface->JoinSession(0, SESSION_NAME_GAME, InviteResult);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Fail to accept invite..."));
	}
}
