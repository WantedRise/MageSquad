// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MSLobbyPlayerState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnLobbyReadyStateChanged,bool);

/**
 * 
 */
UCLASS()
class MAGESQUAD_API AMSLobbyPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	AMSLobbyPlayerState(const FObjectInitializer& ObjectInitializer);

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	virtual void OnRep_PlayerName() override;
public:
	FORCEINLINE bool IsReady() const { return bIsReady; }
	FORCEINLINE bool IsHost() const { return bIsHost; }
	FORCEINLINE const FString& GetUserNickName() const { return UserNickName; }
	
	//서버에서만 지정 가능
	void SetReady(bool bNewReady);
	void SetHost(bool bNewHost);
	void SetUserNickName(const FString& NewNickName);

	FOnLobbyReadyStateChanged OnLobbyReadyStateChanged;
protected:

	class AMSLobbyCharacter* GetLobbyCharacter() const;

	UFUNCTION()
	void OnRep_UserNickName();
	UFUNCTION()
	void OnRep_IsHost();
	UFUNCTION()
	void OnRep_IsReady();
protected:
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_UserNickName)
	FString UserNickName;
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_IsHost)
	uint8 bIsHost : 1;
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_IsReady)
	uint8 bIsReady : 1;
};
