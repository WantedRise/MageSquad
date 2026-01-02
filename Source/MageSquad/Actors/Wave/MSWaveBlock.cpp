// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Wave/MSWaveBlock.h"

// Sets default values
AMSWaveBlock::AMSWaveBlock()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

}

void AMSWaveBlock::ActivateBlock()
{
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
}

void AMSWaveBlock::DeactivateBlock()
{
	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);
}

// Called when the game starts or when spawned
void AMSWaveBlock::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMSWaveBlock::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

