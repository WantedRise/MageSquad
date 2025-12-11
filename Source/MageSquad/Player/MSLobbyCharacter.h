// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MSLobbyCharacter.generated.h"

/*
* 작성자: 이상준
* 작성일: 25/12/07
* 로비에서의 플레이어 캐릭터
* 캐릭터 메쉬 변경 적용할 예정
*/
UCLASS()
class MAGESQUAD_API AMSLobbyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMSLobbyCharacter();
	virtual void PossessedBy(AController* NewController) override;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
protected:
	UFUNCTION()
	void OnRep_UserNickName();
	UFUNCTION()
	void OnRep_IsHostIconVisible();
protected:
	UPROPERTY(EditDefaultsOnly)
	class UWidgetComponent* LobbyPlayerEntryWidgetComponent;
	UPROPERTY()
	class UMSLobbyPlayerEntryWidget* LobbyPlayerEntryWidget;
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_UserNickName)
	FString UserNickName;
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_IsHostIconVisible)
	uint8 bIsHostIconVisible : 1;
};
