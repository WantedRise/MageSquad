// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MS_PlayerCharacter.h"

AMS_PlayerCharacter::AMS_PlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

}

void AMS_PlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

}

void AMS_PlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMS_PlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}
