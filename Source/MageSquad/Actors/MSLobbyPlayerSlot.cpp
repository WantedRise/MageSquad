// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/MSLobbyPlayerSlot.h"
#include "Components/WidgetComponent.h"
#include "Widgets/Lobby/MSLobbyPlayerSlotWidget.h"

// Sets default values
AMSLobbyPlayerSlot::AMSLobbyPlayerSlot()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	PlaySlotWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("PlaySlotWidgetComponent"));

}

void AMSLobbyPlayerSlot::HiddenInviteWidgetComponent()
{
	if (PlaySlotWidgetComponent)
	{
		PlaySlotWidgetComponent->SetVisibility(false);
	}
}

void AMSLobbyPlayerSlot::ShowInviteWidgetComponent()
{
	if (PlaySlotWidgetComponent)
	{
		PlaySlotWidgetComponent->SetVisibility(true);
	}
}

// Called when the game starts or when spawned
void AMSLobbyPlayerSlot::BeginPlay()
{
	Super::BeginPlay();
	
	if (!HasAuthority())
	{
		PlaySlotWidgetComponent->SetVisibility(false);
	}
}


