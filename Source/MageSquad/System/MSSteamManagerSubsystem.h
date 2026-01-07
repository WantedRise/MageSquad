// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "MSSteamManagerSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMSOnCreateSessionCompleteDelegate, bool, bWasSuccessful);

/*
* 작성자: 이상준
* 작성일: 25/12/05
* 스팀 연동 및 세션 생성/참가/초대를 위한 클래스
*/
UCLASS()
class MAGESQUAD_API UMSSteamManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	//멀티플레이어 세션 관리와 관련된 콜백 함수들을 연결
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//등록된 콜백 함수를 모두 해제
	void ClearAllSessionDelegates();
	//세션설정 후 세션 생성
	UFUNCTION(BlueprintCallable, Category = "Network")
	void CreateSteamSession(bool bIsLAN, int32 MaxPlayers);
	UFUNCTION(BlueprintCallable, Category = "Network")
	void DestroySteamSession();
	//친구초대를 위한 스팀오버레이 친구 목록을 띄움
	UFUNCTION(BlueprintCallable, Category = "Network")
	void ShowFriendInvitationScreen();
	//초대받은 세션에 입장 시도
	void TryInvitedJoinSession();
	bool IsSteamConnected();
	int32 GetMaxPlayer();
	
	FMSOnCreateSessionCompleteDelegate MSOnCreateSessionCompleteDelegate;

	//세션의 소유자인지 확인하는 함수
	bool IsPlayerHostingSession();

	void LeaveSession();
protected:

	void OnEndSessionComplete(FName SessionName, bool bWasSuccessful);
	//세션을 삭제하면 메인 메뉴로 이동
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	//세션 참여 후 로비로 이동
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	//현재 세션을 파악하여 초대받은 세션으로 이동
	void OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult);

	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnFindFriendSession(int32 LocalUserNum, bool bWasSuccessful,const TArray<FOnlineSessionSearchResult>& SessionResults);
	
	void OnSessionInviteReceived(const FUniqueNetId& UserId,const FUniqueNetId& FromId, const FString& AppId, const FOnlineSessionSearchResult& InviteResult);
	
protected:
	IOnlineSessionPtr SessionInterface;
	FOnlineSessionSearchResult AcceptedInviteResult;
	

	/*
	FOnSessionUserInviteAcceptedDelegate OnSessionUserInviteAcceptedDelegate;
	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;
	FOnSessionInviteReceivedDelegate OnSessionInviteReceivedDelegate;
	*/
};
