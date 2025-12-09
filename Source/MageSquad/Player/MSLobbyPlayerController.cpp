// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MSLobbyPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "MageSquad.h"


AMSLobbyPlayerController::AMSLobbyPlayerController()
{
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
