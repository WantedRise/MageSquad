// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/MSLobbyPlayerSlot.h"
#include "Components/WidgetComponent.h"

// Sets default values
AMSLobbyPlayerSlot::AMSLobbyPlayerSlot()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = false;
	//PlaySlotWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("PlaySlotWidgetComponent"));
	//PlaySlotWidgetComponent->SetWidgetClass(PlaySlotWidgetClass);
	//PlaySlotWidgetComponent->SetDrawSize(FVector2D(100.0f, 100.0f));
	//PlaySlotWidgetComponent->SetPivot(FVector2D(0.5f, 0.5f));
	//static ConstructorHelpers::FObjectFinder<UMaterial> MaterialFinder(TEXT("/Game/Level/Materials/Loby/Widget3DPassThrough.Widget3DPassThrough"));
	//if (MaterialFinder.Succeeded())
	//{
	//	PlaySlotWidgetComponent->SetMaterial(0, MaterialFinder.Object);
	//}
}

// Called when the game starts or when spawned
void AMSLobbyPlayerSlot::BeginPlay()
{
	Super::BeginPlay();
	
}


