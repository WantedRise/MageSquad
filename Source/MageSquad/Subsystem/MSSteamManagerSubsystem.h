// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "MSSteamManagerSubsystem.generated.h"

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
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Network")
	void CreateSteamSession(bool bIsLAN, int32 MaxPlayers);
	UFUNCTION(BlueprintCallable, Category = "Network")
	void DestroySteamSession();
	UFUNCTION(BlueprintCallable, Category = "Network")
	void ShowFriendInvitationScreen();

protected:
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult);

	void OnSessionInviteReceived(const FUniqueNetId& UserId,const FUniqueNetId& FromId, const FString& AppId, const FOnlineSessionSearchResult& InviteResult);
protected:
	IOnlineSessionPtr SessionInterface;
	/*
	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
	FOnSessionUserInviteAcceptedDelegate OnSessionUserInviteAcceptedDelegate;
	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;
	FOnSessionInviteReceivedDelegate OnSessionInviteReceivedDelegate;
	*/
};
