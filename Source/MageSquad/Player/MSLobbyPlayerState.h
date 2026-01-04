// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MSLobbyPlayerState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnLobbyReadyStateChanged,bool);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCharacterChanged, FName);
 
/*
* 작성자: 이상준
* 작성일: 2025-12-16
* 로비 단계에서 플레이어의 준비 상태, 호스트 여부, 닉네임을 서버 기준으로 동기화하기 위한 PlayerState.
*
* UI 갱신은 Character(로비 캐릭터)에서 처리, PlayerState는 상태만 담당한다.
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
	//로비에 표시될 플레이어 닉네임
	FORCEINLINE const FString& GetUserNickName() const { return UserNickName; }
	
	//서버에서만 지정 가능
	void SetReady(bool bNewReady);
	void SetHost(bool bNewHost);
	void SetUserNickName(const FString& NewNickName);

	//준비 상태 변경 시 캐릭터/UI에 알리기 위한 델리게이트
	FOnLobbyReadyStateChanged OnLobbyReadyStateChanged;

	FOnCharacterChanged OnCharacterChanged;

	void SetSelectedCharacter(FName CharacterID);

	UFUNCTION()
	void OnRep_SelectedCharacterID();
protected:

	class AMSLobbyCharacter* GetLobbyCharacter() const;

	UFUNCTION()
	void OnRep_UserNickName();
	UFUNCTION()
	void OnRep_IsHost();
	UFUNCTION()
	void OnRep_IsReady();


	
protected:
	// 로비 UI에 표시될 플레이어 닉네임 (서버에서 설정, 클라이언트 동기화)
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_UserNickName)
	FString UserNickName;
	// 로비 방장 여부 (서버 권한)
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_IsHost)
	uint8 bIsHost : 1;
	// 로비 준비 상태 (Ready / Not Ready)
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_IsReady)
	uint8 bIsReady : 1;

	UPROPERTY(ReplicatedUsing = OnRep_SelectedCharacterID)
	FName SelectedCharacterID;

};
