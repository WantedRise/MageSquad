// Fill out your copyright notice in the Description page of Project Settings.


#include "System/MSSteamManagerSubsystem.h"
#include "OnlineSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "GameFramework/PlayerController.h"
#include "MageSquad.h"

const FName SESSION_NAME_GAME = FName(TEXT("MSSteamSeesion"));

void UMSSteamManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		SessionInterface = OnlineSub->GetSessionInterface();
		//멀티플레이어 세션 관리와 관련된 콜백 함수들을 연결
		if (SessionInterface.IsValid())
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UMSSteamManagerSubsystem::OnCreateSessionComplete);
			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UMSSteamManagerSubsystem::OnDestroySessionComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UMSSteamManagerSubsystem::OnJoinSessionComplete);
			SessionInterface->OnSessionUserInviteAcceptedDelegates.AddUObject(this, &UMSSteamManagerSubsystem::OnSessionUserInviteAccepted);
			SessionInterface->OnSessionInviteReceivedDelegates.AddUObject(this, &UMSSteamManagerSubsystem::OnSessionInviteReceived);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UMSSteamManagerSubsystem::OnFindSessionsComplete);
			SessionInterface->OnFindFriendSessionCompleteDelegates[0].AddUObject(this, &UMSSteamManagerSubsystem::OnFindFriendSession);
			SessionInterface->OnEndSessionCompleteDelegates.AddUObject(this, &UMSSteamManagerSubsystem::OnEndSessionComplete);
		}
	}
}

void UMSSteamManagerSubsystem::Deinitialize()
{
	ClearAllSessionDelegates();
}

void UMSSteamManagerSubsystem::ClearAllSessionDelegates()
{
	if (SessionInterface && SessionInterface.IsValid())
	{
		SessionInterface->DestroySession(SESSION_NAME_GAME);
		SessionInterface->OnCreateSessionCompleteDelegates.RemoveAll(this);
		SessionInterface->OnDestroySessionCompleteDelegates.RemoveAll(this);
		SessionInterface->OnFindSessionsCompleteDelegates.RemoveAll(this);
		SessionInterface->OnJoinSessionCompleteDelegates.RemoveAll(this);
		SessionInterface->OnSessionUserInviteAcceptedDelegates.RemoveAll(this);
		SessionInterface->OnSessionInviteReceivedDelegates.RemoveAll(this);
		SessionInterface->OnFindFriendSessionCompleteDelegates[0].RemoveAll(this);
		SessionInterface->OnEndSessionCompleteDelegates.RemoveAll(this);
	}
}

void UMSSteamManagerSubsystem::OnEndSessionComplete(FName SessionName, bool bWasSuccessful)
{

}

void UMSSteamManagerSubsystem::CreateSteamSession(bool bIsLAN, int32 MaxPlayers)
{
	if (SessionInterface.IsValid())
	{

		FOnlineSession* CurrentSession = SessionInterface->GetNamedSession(SESSION_NAME_GAME);
		//현재 참여중
		if (CurrentSession != nullptr)
		{
			OnCreateSessionComplete(NAME_None, false);
			UE_LOG(LogTemp, Log, TEXT("Player is already in the InviteResult session (ID: %s)."), *CurrentSession->GetSessionIdStr());
			return;
		}

		FOnlineSessionSettings SessionSettings;
		SessionSettings.bIsLANMatch = false;
		SessionSettings.bUsesPresence = true;
		SessionSettings.NumPublicConnections = MaxPlayers;
		SessionSettings.bAllowJoinViaPresence = true;
		SessionSettings.bIsDedicated = false;        // 전용 서버가 아님
		SessionSettings.bAllowJoinInProgress = true; // 게임 중 참여 허용
		SessionSettings.bShouldAdvertise = true;     // 친구들이 세션을 찾을 수 있도록 알림
		SessionSettings.bUseLobbiesIfAvailable = true; // 로비 사용
		//친구 초대를 위한 설정
		SessionSettings.Set(FName("Session Type"), FString("AdvertisedSessionHost"), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing); 
		SessionInterface->CreateSession(0, SESSION_NAME_GAME, SessionSettings);
	}
}

void UMSSteamManagerSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogMSNetwork, Warning, TEXT("%s Session  created successfully!"),*SessionName.ToString());
	}
	MSOnCreateSessionCompleteDelegate.Broadcast(bWasSuccessful);
}

void UMSSteamManagerSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogMSNetwork, Warning, TEXT("OnFindSessionsComplete true"));

	}
	else
	{
		UE_LOG(LogMSNetwork, Warning, TEXT("OnFindSessionsComplete false"));
	}
}

void UMSSteamManagerSubsystem::OnFindFriendSession(int32 LocalUserNum, bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& SessionResults)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogMSNetwork, Warning, TEXT("OnFindFriendSession true"));
		SessionInterface->JoinSession(0, SESSION_NAME_GAME, SessionResults[0]);
	}
	else
	{
		UE_LOG(LogMSNetwork, Warning, TEXT("OnFindFriendSession false"));
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
	//필요없는 코드
	UE_LOG(LogMSNetwork, Warning, TEXT("Session '%s' OnDestroySessionComplete! %s"), *SessionName.ToString(),bWasSuccessful?TEXT("true"):TEXT("false"));
	if (bWasSuccessful && AcceptedInviteResult.IsValid())
	{
		// 여기서 메인메뉴로 가지 말고 바로 Join
		if (SessionInterface->JoinSession(0, SESSION_NAME_GAME, AcceptedInviteResult))
		{
			// 성공적으로 요청을 보냈으므로 리셋
			AcceptedInviteResult = FOnlineSessionSearchResult();
		}
		return;
	}

	//UGameplayStatics::OpenLevel(GetWorld(), TEXT("MainmenuLevel"));
}

void UMSSteamManagerSubsystem::ShowFriendInvitationScreen()
{
	UE_LOG(LogMSNetwork, Warning, TEXT("ShowFriendInvitationScreen"));
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		UE_LOG(LogMSNetwork, Warning, TEXT("OnlineSub"));
		if (IOnlineExternalUIPtr UIPtr = OnlineSub->GetExternalUIInterface())
		{
			if (!SessionInterface.IsValid())
			{
				UE_LOG(LogMSNetwork, Warning, TEXT("NULL SessionInterface"));
				return;
			}
			if (nullptr == SessionInterface->GetNamedSession(SESSION_NAME_GAME))
			{
				UE_LOG(LogMSNetwork, Warning, TEXT("NULL GetNamedSession"));
				return;
			}
			//친구초대를 위한 스팀오버레이 친구 목록을 띄움
			UIPtr->ShowInviteUI(0, SESSION_NAME_GAME);
		}
	}
}

void UMSSteamManagerSubsystem::TryInvitedJoinSession()
{
	if (SessionInterface && SessionInterface.IsValid() && AcceptedInviteResult.IsValid())
	{
		SessionInterface->JoinSession(0, SESSION_NAME_GAME, AcceptedInviteResult);
	}
}

void UMSSteamManagerSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	FString TravelURL;
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PlayerController && SessionInterface->GetResolvedConnectString(SessionName, TravelURL))
	{
		AcceptedInviteResult = FOnlineSessionSearchResult();

		//steam.76561198046755719:7777 to steam.76561198046755719
		//포트번호가 붙어있으면 실행 안되는 것으로 보임
		TravelURL.RemoveAt(TravelURL.Find(":"), 5);
		//로비로 이동하기 위해
		FString MapPath = TEXT("/Game/Level/LobbyLevel");
		TravelURL += MapPath;
		
		UE_LOG(LogMSNetwork, Warning, TEXT("ClientTravel. Joining session Complete... %s"),*TravelURL);
		PlayerController->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
	}
	else
	{
		UE_LOG(LogMSNetwork, Warning, TEXT("Joining session Fail..."));
	}
}

void UMSSteamManagerSubsystem::OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult)
{
	if (bWasSuccessful)
	{
		if (!SessionInterface.IsValid())
		{
			UE_LOG(LogMSNetwork, Warning, TEXT("SessionInterface UnVaild"));
			return;
		}

		FOnlineSession* CurrentSession = SessionInterface->GetNamedSession(SESSION_NAME_GAME);

		//현재 참여중인 세션이 없으면 참여하기
		if (CurrentSession == nullptr)
		{
			UE_LOG(LogMSNetwork, Warning, TEXT("OnSessionUserInviteAccepted Invite accepted. Joining session..."));
			SessionInterface->JoinSession(0, SESSION_NAME_GAME, InviteResult);
			return;
		}

		//이미 참여하고 있는 세션이라면 리턴
		if (CurrentSession->GetSessionIdStr() == InviteResult.GetSessionIdStr())
		{
			UE_LOG(LogTemp, Log, TEXT("Player is already in the InviteResult session (ID: %s)."), *CurrentSession->GetSessionIdStr());
			return;
		}

		//참여중인 세션을 종료하고 초대받은 세션을 저장
		AcceptedInviteResult = InviteResult;
		DestroySteamSession();

		//서버 플레이어가 Destroy Session을 호출하면 세션 종료 -> 모두가 나가짐
		//클라이언트 플레이어가 Destroy Session을 호출하면 어떻게 되는지 모르겠음
		//Todo : LSJ - Destroy Session 차이점을 파악하고 로직개선하기
		/* 세션 소유자인지 판단하고 소유자라면 세션 destroy
		if (IsPlayerHostingSession())
		{
			UE_LOG(LogMSNetwork, Warning, TEXT("OnSessionUserInviteAccepted - Current session match the invited session"));
		}
		else //소유자가 아니므로 세션 종료 후 이동
		{
			UE_LOG(LogTemp, Log, TEXT("OnSessionUserInviteAccepted Current session (ID: %s) does not match the invited session (ID: %s)."), *CurrentSession->GetSessionIdStr(), *InviteResult.GetSessionIdStr())
		}
		*/
	}
}

bool UMSSteamManagerSubsystem::IsPlayerHostingSession()
{
	if (SessionInterface==nullptr || !SessionInterface.IsValid())
	{
		return false;
	}

	// 기본 세션 이름인 NAME_GameSession을 사용하여 현재 세션을 가져옴
	FOnlineSession* CurrentSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (CurrentSession == nullptr)
	{
		return false; // 참여 중인 세션이 없음
	}

	//세션의 소유자인지 확인
	FNamedOnlineSession* NamedOnlineSession = SessionInterface->GetNamedSession(SESSION_NAME_GAME);
	if (NamedOnlineSession)
	{
		const FUniqueNetIdPtr LocalUserId = NamedOnlineSession->LocalOwnerId;
		if (LocalUserId->IsValid() && CurrentSession->OwningUserId.IsValid())
		{
			return LocalUserId == CurrentSession->OwningUserId;
		}
	}

	return false;
}

void UMSSteamManagerSubsystem::OnSessionInviteReceived(const FUniqueNetId& UserId, const FUniqueNetId& FromId, const FString& AppId, const FOnlineSessionSearchResult& InviteResult)
{
	UE_LOG(LogMSNetwork, Warning, TEXT("OnSessionInviteReceived %s "),*AppId);
}

bool UMSSteamManagerSubsystem::IsSteamConnected()
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub)
	{
		// 현재 기본 플랫폼 서비스가 "Steam"인지
		if (OnlineSub->GetSubsystemName() == STEAM_SUBSYSTEM)
		{
			return true;
		}
	}

	return false; // 스팀 서브시스템이 없거나 로그인되지 않음
}

int32 UMSSteamManagerSubsystem::GetMaxPlayer()
{
	if (!IsSteamConnected())
	{
		return 1;
	}
	
    if (SessionInterface.IsValid())
    {
        FNamedOnlineSession* CurrentSession = SessionInterface->GetNamedSession(NAME_GameSession);
        if (CurrentSession)
        {
            // 세션 설정에 저장된 최대 인원수 반환
            return CurrentSession->SessionSettings.NumPublicConnections;
        }
    }
	return 1;
}

void UMSSteamManagerSubsystem::LeaveSession()
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (!Subsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("LeaveSession: OnlineSubsystem not found"));
		return;
	}

	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("LeaveSession: SessionInterface invalid"));
		return;
	}

	// 세션이 존재하지 않으면 그냥 무시
	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(NAME_GameSession);
	if (!Session)
	{
		UE_LOG(LogTemp, Log, TEXT("LeaveSession: No active session"));
		return;
	}

	// ⭐ Client는 DestroySession 호출해도 문제 없음
	SessionInterface->DestroySession(NAME_GameSession);

	UE_LOG(LogTemp, Log, TEXT("LeaveSession: DestroySession requested"));
}