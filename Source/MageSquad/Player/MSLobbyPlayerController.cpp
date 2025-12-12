// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MSLobbyPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "MageSquad.h"
#include "GameFramework/PlayerState.h"

AMSLobbyPlayerController::AMSLobbyPlayerController()
{
	//카메라 시점 제어를 위한 설정
	bAutoManageActiveCameraTarget = false;
}

void AMSLobbyPlayerController::OnPossess(APawn* NewPawn)
{
	Super::OnPossess(NewPawn);

}

void AMSLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsLocalController())
	{
		FName TargetTag = TEXT("LobbyCamera");

		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), TargetTag, FoundActors);

		if (FoundActors.Num() > 0)
		{
			//LobbyCamera 액터로 시점 전환
			SetViewTargetWithBlend(FoundActors[0]);
		}

		SetShowMouseCursor(true);
	}
}
