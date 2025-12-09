// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MSLobbyCharacter.h"

// Sets default values
AMSLobbyCharacter::AMSLobbyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AMSLobbyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	
}

void AMSLobbyCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);


}

// Called every frame
void AMSLobbyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMSLobbyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

